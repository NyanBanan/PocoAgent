#ifndef _PCAPSTATISTICS
#define _PCAPSTATISTICS

//Добавление пакета в таблицу статистики
bool addPacket(const u_char * packet);

//Очистка таблицы статистики 
bool clearMap();

//Запуск процесса сбора статистики
void startStatisticsDaemon();

//Остановка процесса сбора статистики
void stopStatisticsDaemon();

#endif