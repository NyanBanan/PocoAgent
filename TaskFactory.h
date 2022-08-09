#ifndef _TASKFACTORY
#define _TASKFACTORY

#include "Poco/Task.h"
#include "Poco/TaskManager.h"

//Запустить новый поток
void startTask(Poco::Task * task);

//Завершить все потоки
void cancelAllTasks();

//Дождаться завершения всех потоков
void joinAllTasks();

#endif