#include <unordered_map>
#include <map>
#include <ctime>

#include "pcap.h"

#include "Poco/Mutex.h"

#include "../../TaskFactory.h"
#include "PcapStatistics.h"
#include "../../Logger.h"

using namespace std;
using namespace Poco;

#define MAX_MAP_SIZE 4000
#define DAEMON_SLEEP_TIME 1000

#define SIZE_ETHERNET 14
/* Ethernet адреса состоят из 6 байт */
#define ETHER_ADDR_LEN 6

/* Заголовок Ethernet */
struct sniff_ethernet {
u_char ether_dhost[ETHER_ADDR_LEN]; /* Адрес назначения */
u_char ether_shost[ETHER_ADDR_LEN]; /* Адрес источника */
u_short ether_type; /* IP? ARP? RARP? и т.д. */
};

/* IP header */
struct sniff_ip {
u_char ip_vhl;  /* версия << 4 | длина заголовка >> 2 */
u_char ip_tos;  /* тип службы */
u_short ip_len;  /* общая длина */
u_short ip_id;  /* идентефикатор */
u_short ip_off;  /* поле фрагмента смещения */
#define IP_RF 0x8000  /* reserved флаг фрагмента */
#define IP_DF 0x4000  /* dont флаг фрагмента */
#define IP_MF 0x2000  /* more флаг фрагмента */
#define IP_OFFMASK 0x1fff /* маска для битов фрагмента */
u_char ip_ttl;  /* время жизни */
u_char ip_p;  /* протокол */
u_short ip_sum;  /* контрольная сумма */
in_addr ip_src,ip_dst; /* адрес источника и адрес назначения */
};
#define IP_HL(ip)  (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)  (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
    u_short th_sport; /* порт источника */
    u_short th_dport; /* порт назначения */
    tcp_seq th_seq;  /* номер последовательности */
    tcp_seq th_ack;  /* номер подтверждения */
    u_char th_offx2; /* смещение данных, rsvd */
    #define TH_OFF(th) (((th)->th_offx2 & 0xf0) >> 4)
    u_char th_flags;
    #define TH_FIN 0x01
    #define TH_SYN 0x02
    #define TH_RST 0x04
    #define TH_PUSH 0x08
    #define TH_ACK 0x10
    #define TH_URG 0x20
    #define TH_ECE 0x40
    #define TH_CWR 0x80
    #define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short th_win;  /* окно */
    u_short th_sum;  /* контрольная сумма */
    u_short th_urp;  /* экстренный указатель */
};

u_short swapUShort(u_short x)   {  return (x & 0xFF) << 8 | (x & 0xFF00) >>  8;  }

void logIP(const sniff_ip * p)
{
    log_information("Source: "+to_string(p->ip_src.S_un.S_un_b.s_b1)+"."
                            +to_string(p->ip_src.S_un.S_un_b.s_b2)+"."
                            +to_string(p->ip_src.S_un.S_un_b.s_b3)+"."
                            +to_string(p->ip_src.S_un.S_un_b.s_b4));
    log_information("Destination: "+to_string(p->ip_dst.S_un.S_un_b.s_b1)+"."
                            +to_string(p->ip_dst.S_un.S_un_b.s_b2)+"."
                            +to_string(p->ip_dst.S_un.S_un_b.s_b3)+"."
                            +to_string(p->ip_dst.S_un.S_un_b.s_b4));
}

void logTCP(const sniff_tcp * packet)
{
    log_information("Source port: "+to_string((packet->th_sport)));
    log_information("Destination port: "+to_string(swapUShort(packet->th_dport)));
}

struct PacketKeyTime
{
    u_char  protocol;       //код протокола tcp, udp и т.п.
    u_short portSrc;        //порт источника
    u_short portDst;        //целевой порт
    in_addr ipSrc;          //адрес источника
    in_addr ipDst;          //целевой адрес
    time_t creation_time;   //время создания записи
};

struct PacketKey
{
    u_char  protocol;   //код протокола tcp, udp и т.п.
    u_short portSrc;    //порт источника
    u_short portDst;    //целевой порт
    in_addr ipSrc;      //адрес источника
    in_addr ipDst;      //целевой адрес
};

struct PacketStatistics
{
    u_char status;              //кол-во фиксаций статистики
    u_short min_packet_size;    //минимальный размер пакета
    u_short max_packet_size;    //максимальный размер пакета
    u_short avg_packet_size;    //средний размер пакета
    u_long total_packet_size;   //общий размер пакетов
    u_long packet_count;               //кол-во пакетов
    time_t last_update;         //время последнего изменения статистики
};

//Хэш функция для ключа таблицы
struct HashPacketKey 
{
    size_t operator()(const PacketKey &key ) const
    {
        return hash<u_char>()(key.protocol) ^ 
                hash<u_short>()(key.portSrc) ^
                hash<u_short>()(key.portDst) ^
                hash<u_char>()(key.ipSrc.S_un.S_un_b.s_b1) ^
                hash<u_char>()(key.ipSrc.S_un.S_un_b.s_b2) ^
                hash<u_char>()(key.ipSrc.S_un.S_un_b.s_b3) ^
                hash<u_char>()(key.ipSrc.S_un.S_un_b.s_b4) ^
                hash<u_long>()(key.ipSrc.S_un.S_addr) ^
                hash<u_short>()(key.ipSrc.S_un.S_un_w.s_w1) ^
                hash<u_short>()(key.ipSrc.S_un.S_un_w.s_w2) ^
                hash<u_char>()(key.ipDst.S_un.S_un_b.s_b1) ^
                hash<u_char>()(key.ipDst.S_un.S_un_b.s_b2) ^
                hash<u_char>()(key.ipDst.S_un.S_un_b.s_b3) ^
                hash<u_char>()(key.ipDst.S_un.S_un_b.s_b4) ^
                hash<u_long>()(key.ipDst.S_un.S_addr) ^
                hash<u_short>()(key.ipDst.S_un.S_un_w.s_w1) ^
                hash<u_short>()(key.ipDst.S_un.S_un_w.s_w2);
    }
};

//Проверка на равенство для ключа таблицы
struct EqualToPacketKey 
{
    bool operator()(const PacketKey &a,const PacketKey &b ) const
    {
        if  (a.ipSrc.S_un.S_un_b.s_b1 == b.ipSrc.S_un.S_un_b.s_b1 &&
                a.ipSrc.S_un.S_un_b.s_b2 == b.ipSrc.S_un.S_un_b.s_b2 &&
                a.ipSrc.S_un.S_un_b.s_b3 == b.ipSrc.S_un.S_un_b.s_b3 &&
                a.ipSrc.S_un.S_un_b.s_b4 == b.ipSrc.S_un.S_un_b.s_b4 &&
                a.ipSrc.S_un.S_un_w.s_w1 == b.ipSrc.S_un.S_un_w.s_w1 &&
                a.ipSrc.S_un.S_un_w.s_w2 == b.ipSrc.S_un.S_un_w.s_w2 &&
                a.ipSrc.S_un.S_addr == b.ipSrc.S_un.S_addr &&

                a.ipDst.S_un.S_un_b.s_b1 == b.ipDst.S_un.S_un_b.s_b1 &&
                a.ipDst.S_un.S_un_b.s_b2 == b.ipDst.S_un.S_un_b.s_b2 &&
                a.ipDst.S_un.S_un_b.s_b3 == b.ipDst.S_un.S_un_b.s_b3 &&
                a.ipDst.S_un.S_un_b.s_b4 == b.ipDst.S_un.S_un_b.s_b4 &&
                a.ipDst.S_un.S_un_w.s_w1 == b.ipDst.S_un.S_un_w.s_w1 &&
                a.ipDst.S_un.S_un_w.s_w2 == b.ipDst.S_un.S_un_w.s_w2 &&
                a.ipDst.S_un.S_addr == b.ipDst.S_un.S_addr &&

                a.portSrc == b.portSrc &&
                a.portDst == b.portDst &&
                a.protocol == b.protocol)
                return 1;
        return 0;
    }
};

struct CompPacketKey 
{
    bool operator()(const PacketKeyTime &a,const PacketKeyTime &b ) const
    {
        return a.creation_time > b.creation_time;
    }
};

//std::unordered_map<PacketKey,PacketStatistics,HashPacketKey,EqualToPacketKey> packetsMap;

//Флаг блокировки доступа к ьаблице на время удаления записей
//bool isMapLocked = false;

static Mutex mapMutex;

static map<PacketKeyTime,PacketStatistics,CompPacketKey> packetsMap;

/* void lockMap()
{
    while (isMapLocked)
        continue;
    isMapLocked = true;
}

void unlockMap()
{
    isMapLocked = false;
} */

bool sendStatistics(const pair<PacketKeyTime,PacketStatistics> &kv_pair)
{
    log_information("Sent information!");
    return false;
}

//Парсинг pcap пакета
pair<sniff_ip *,sniff_tcp*> parsePacket(const u_char * packet)
{
    sniff_ethernet * ethernet;      /* Заголовок Ethernet */
    sniff_ip * ip;                  /* Заголовок IP */
    sniff_tcp * tcp;                /* Заголовок TCP */
    //char * payload;                 /* Данные пакета */

    u_int size_ip;
    u_int size_tcp;

    ethernet = (sniff_ethernet*)(packet); 
    int temp = ethernet->ether_type;
    if (temp != 8)
    {
        //log_information("Not IP type of packet");
        return {NULL,NULL};
    }
    ip = (sniff_ip*)(packet + SIZE_ETHERNET);
    //log_information("loaded ip");
    size_ip = IP_HL(ip)*4;
    if (size_ip < 20) 
    {
        //log_information("   * Invalid IP header length: " + to_string(size_ip));
        return {NULL,NULL};
    }
    //logIP(ip);
    tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
    //log_information("loaded tcp");
    size_tcp = TH_OFF(tcp)*4;
    if (size_tcp < 20) 
    {
        //log_information("   * Invalid TCP header length: " + to_string(size_tcp));
        return {NULL,NULL};
    }
    //logTCP(tcp);
    //payload = (char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
    return {ip,tcp};
}

//Добавление пакета в таблицу статистики
bool addPacket(const u_char * packet)
{
    auto ip_tcp_pair = parsePacket(packet);
    if (!ip_tcp_pair.first)
        return 0;
    
    auto ip = ip_tcp_pair.first;
    auto tcp = ip_tcp_pair.second;

    //Ключ
    /* PacketKey key {  ip->ip_p, 
                    tcp->th_sport, 
                    tcp->th_dport, 
                    ip->ip_src, 
                    ip->ip_dst   }; */
    PacketKeyTime key {  ip->ip_p, 
                    tcp->th_sport, 
                    tcp->th_dport, 
                    ip->ip_src, 
                    ip->ip_dst,
                    time(0)    };

    //Имеющееся в таблице значение
    auto stat = packetsMap[key];

    //Создание новой записи
    if (!stat.packet_count)
    {
        packetsMap[key] = {0,
                            ip->ip_len,
                            ip->ip_len,
                            ip->ip_len,
                            ip->ip_len,
                            1,
                            time(0)};
        log_information("Added new statistics "+to_string(packetsMap.size())+ "/"+to_string(MAX_MAP_SIZE));
        
        if (packetsMap.size() == MAX_MAP_SIZE)
        {
            Mutex::ScopedLock lock(mapMutex);
            auto last = *(--packetsMap.end());
            sendStatistics(last);
            packetsMap.erase(last.first);
            Mutex::ScopedLock unlock(mapMutex);
            log_information("Deleted statistics "+to_string(packetsMap.size())+"/"+to_string(MAX_MAP_SIZE));
        }
    }
    else
    {
        stat.packet_count += 1;
        stat.total_packet_size += ip->ip_len;
        stat.min_packet_size = min(stat.min_packet_size,ip->ip_len);
        stat.max_packet_size = max(stat.max_packet_size,ip->ip_len);
        stat.avg_packet_size = stat.total_packet_size / stat.packet_count;
        stat.last_update = time(0);
        packetsMap[key] = stat;
    }
    return 1;
}

//Очистка таблицы статистики 
bool clearMap()
{
    packetsMap.clear();
}

class StatisticDaemonTask: public Poco::Task
    {
    public:
        int status = 0;

        StatisticDaemonTask(): Task("StatisticDaemonTask") 
        { }

        //Запуск задачи
        void runTask()
        {
            log_information("Starting statistics daemon!");
            status = 1;
            checkStatisticsLoop();
        }

        void checkStatisticsLoop()
        {
            vector<PacketKeyTime> to_delete;
            while(status)
            {
                //lockMap();
                Mutex::ScopedLock lock(mapMutex);
                for (auto kv_pair : packetsMap)
                {
                    auto key = kv_pair.first;
                    auto value = kv_pair.second;

                    if (!value.last_update)
                        continue;
                    auto lifetime = time(0) - value.last_update;
                    
                    if (    lifetime >= 12 && lifetime < 28 &&  value.status == 0    ||
                            lifetime >= 28 && lifetime < 78 &&  value.status == 1    ||
                            lifetime >= 78 && lifetime < 128 &&  value.status == 2   )
                    {
                        sendStatistics(kv_pair);
                        value.status++;
                        packetsMap[key] = value;
                    }
                    else if (lifetime >= 128)
                    {
                        sendStatistics(kv_pair);
                        to_delete.push_back(key);
                    }
                }
                for (auto x: to_delete)
                {
                    packetsMap.erase(x);
                    log_information("Deleted statistics "+to_string(packetsMap.size())+"/"+to_string(MAX_MAP_SIZE));
                }
                to_delete.clear();
                //unlockMap();
                Mutex::ScopedLock unlock(mapMutex);
                sleep(DAEMON_SLEEP_TIME);
            }

            log_information("Exited statistics daemon loop!");
        }
    };

StatisticDaemonTask * stat_daemon_task;

//Запуск процесса сбора статистики
void startStatisticsDaemon()
{
    stat_daemon_task = new StatisticDaemonTask;
    startTask(stat_daemon_task);
}

//Остановка процесса сбора статистики
void stopStatisticsDaemon()
{
    stat_daemon_task->status = 0;
}