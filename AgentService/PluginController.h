#pragma once

#include "LibraryLoader.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"

class PluginTask:public Poco::Task
{
private:
    ISender* pPlugin;
public:
    PluginTask(ISender* _p):Task("GetInterfaces"){
        pPlugin=_p;
    }

    ~PluginTask(){
    }

    void runTask(){
        Poco::Util::Application& app = Poco::Util::Application::instance();
        while(!isCancelled()){
            log_information("Service is alive " + Poco::DateTimeFormatter::format(app.uptime()));
            log_information(pPlugin->routine());
            sleep(5000);
        }
        pPlugin->clear();
    }
};

class PluginTaskController
{
private:
    Poco::TaskManager pm;
    ISender* pPlugin;
	std::string status= "stopped";
public:
    PluginTaskController(ISender* _pPlugin){
        pPlugin=_pPlugin;
    }

    void startPlugin(){
        if(pPlugin!=nullptr){
            if(status!="starting" && status!="started"){
                status = "starting";
                pm.start(new PluginTask(pPlugin));
                log_information("Started!");
                status="started";
            }
        }
        else
            log_information("plugin not working");
    }
    void stopPlugin(){
        if(pPlugin!=nullptr){
            if(status!="stopping" && status!="stopped"){
                status = "stopping";
                pm.cancelAll();
                log_information("Stopped!");
                status = "stopped";
            }
        }
        else
            log_information("plugin not working");
    }
    std::string getStatus(){
        return status;
    }

    std::string getPluginInterfacesString(){
        if(pPlugin!=nullptr)
            return pPlugin->getInterfacesString();
        else
            return "\"plugin doesnt work\"";
    }
};
