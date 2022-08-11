#define SCAN_INTERVAL 3000

#include <iostream>

#include "Poco/ClassLibrary.h"
//#include "Poco/Task.h"
//#include "Poco/TaskManager.h"
//#include "Poco/ThreadPool.h"

#include "../AbstractPlugin.h"
#include "../../../../../ngidsagentmod/Logger.h"
#include "PcapMethods.h"
#include "../../TaskFactory.h"


class PluginGetInterfaces: public AbstractPlugin
{
    class GetInterfaceTask: public PluginTask
    {
    public:
        GetInterfaceTask(): PluginTask("PluginGetInterfaces")
        {

        }

        //Запуск задачи
        void runTask() override
        {
            log_information("Starting capturing at runTusk");
            startPacketsCapturing();
        }
    };

public:

    void start()
    {
        int_task = new GetInterfaceTask;
        startTask(int_task);
        log_information("Capturing started!");
    }

    void stop()
    {
        stopPacketsCapturing();
        log_information("Capturing stopped!");
    }

    std::string getStatus()
    {
        return getPcapStatus();
    }

    std::string getData(){
        std::string str;
        auto temp = getAllDevs();
        std::string ret="[\n";
        auto end=temp.end()-=1;
        for (auto t=temp.begin();t!=end;++t){
            str=*t;
            ret+="\""+str+"\",\n";
        }
        str=*end;
        ret+="\""+str+"\"\n]";
        std::replace(ret.begin(),ret.end(),'\\','/');
        log_information(ret);
        return ret;
    }
};




POCO_BEGIN_MANIFEST(AbstractPlugin)
POCO_EXPORT_CLASS(PluginGetInterfaces)
POCO_END_MANIFEST

// optional set up and clean up functions
void pocoInitializeLibrary()
{
    //std::cout << "PluginLibrary initializing" << std::endl;
}
void pocoUninitializeLibrary()
{
    //std::cout << "PluginLibrary uninitializing" << std::endl;
}     