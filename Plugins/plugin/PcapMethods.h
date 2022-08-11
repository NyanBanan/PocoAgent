#ifndef _PCAPMETHODS
#define _PCAPMETHODS

#include <string>
#include <vector>
#include "PcapStatistics.h"



//Получить список всех сетевых интерфейсов
std::vector<char *> getAllDevs();

//Запуск получения пакетов
void startPacketsCapturing();

//Остановка полученяи пакетов
void stopPacketsCapturing();

//Получить сетевой интерфейс (pcap выбирает самостоятельно)
char* getDev();

//Получить текущий статус захвата
std::string getPcapStatus();

#endif
