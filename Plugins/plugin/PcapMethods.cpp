#include "pcap.h"

#include "../../Logger.h"
#include "PcapMethods.h"



using namespace std;

int pcap_status = 4;
pcap_t * handle;

/* u_short swapUShort(u_short x)   {  return (x & 0xFF) << 8 | (x & 0xFF00) >>  8;  }

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
} */

vector<char *> getAllDevs()
{
    char error[PCAP_ERRBUF_SIZE];
    pcap_if_t *interfaces,*temp;

    if(pcap_findalldevs(&interfaces,error)==-1)
    {
        log_error("Find All Devs Error: "+string(error));
        return {};   
    }
    vector<char *> res;
    for(temp=interfaces;temp;temp=temp->next)
    {
        res.push_back(temp->name);
    }
    
    //pcap_freealldevs(interfaces);
    return res;
}

char * getDev()
{
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) 
    {
        log_error("Couldn't find default device: "+ string(errbuf));
        return "";
    }

    return dev;
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    if (!packet)
        return;
    log_information("Jacked a packet with length of "+to_string(header->len));
}

void custom_loop()
{
    pcap_pkthdr header;
    const u_char *packet;
    clearMap();
    startStatisticsDaemon();
    while (true)
    {
        if (pcap_status == 3)
            return;
        packet = pcap_next(handle, &header);
        if (!packet ||  header.len == 0)
            continue;
        //log_information("Jacked a packet with length of "+to_string(header.len));

        addPacket(packet);
    }
}

int catchPackets(const char * dev, int countPackets = -1)
{
    char errbuf[PCAP_ERRBUF_SIZE]; /* Строка для хранения ошибки */
    bpf_program fp;  /* Скомпилированный фильтр */
    char filter_exp[] = "port 23"; /* Выражение фильтра */
    bpf_u_int32 mask;  /* Сетевая маска */
    bpf_u_int32 net;  /* IP */
    pcap_pkthdr header; /* Заголовок который нам дает PCAP */
    const u_char *packet;  /* Пакет */

    log_information("Captured dev: "+ string(dev));

    /* Определение свойств устройства */
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) 
    {
        log_warning("Couldn't get netmask for device " + string(dev) +" : " + string(errbuf));
        net = 0;
        mask = 0;
    }

    /* Создание сессии в неразборчивом режиме */
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) 
    {
        log_error("Couldn't open device " + string(dev) +" : " + string(errbuf));
        return(2);
    }

    log_information("Session opened!");

    //Настройка фильтров
    /* 
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) 
    {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
    }

    if (pcap_setfilter(handle, &fp) == -1) 
    {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
    }
     */
    pcap_status = 2;
    custom_loop();
    /* Закрытие сессии */
    pcap_close(handle);
    pcap_status = 4;
    log_information("Session closed!");
    return 0;
}

void startPacketsCapturing()
{
    pcap_status = 1;
    //auto dev = getDev();      //Интерфейс по умолчанию почти не получает IP пакеты
    //catchPackets(dev);

    auto lst = getAllDevs();
    catchPackets(lst[6]);     //На моём компьютере тут много IP пакетов
}


void stopPacketsCapturing()
{
    pcap_status = 3;
    stopStatisticsDaemon();
    log_information("Stopping capturing!");
}

std::string getPcapStatus()
{
    switch (pcap_status)
    {
        case 1: return "starting";
        case 2: return "started";
        case 3: return "stopping";
        case 4: return "stopped";
    }
}