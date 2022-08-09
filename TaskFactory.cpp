
#include "TaskFactory.h"

using namespace Poco;

TaskManager tm;

//Запустить новый поток
void startTask(Task * task)
{
    tm.start(task);
}

//Завершить все потоки
void cancelAllTasks()
{
    tm.cancelAll();
}

//Дождаться завершения всех потоков
void joinAllTasks()
{
    tm.joinAll();
}