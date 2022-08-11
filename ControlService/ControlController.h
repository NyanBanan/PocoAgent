
#pragma once
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Control.h"
class ControlTask : public Poco::Task
{
private:
    Control* pControl;
public:
    ControlTask(Control* _c):Task("ControlPocoAgent"){
        pControl=_c;
    }
    void runTask(){
        while(!isCancelled()){
            //log_information("Starting agent");
            pControl->startAgent();
            sleep(10000);
        }
    }
};

class ControlTaskController{
private:
    Poco::TaskManager pm;
    Control *const pControl;
    std::string status= "stopped";
public:
    explicit ControlTaskController(Control *const _c) : pControl(_c) {
    }

    ~ControlTaskController(){
        stopControlTask();
        pm.joinAll();
    }

    void startControlTask(){
        if(status!="starting" && status!="started"){
            status = "starting";
            pm.start(new ControlTask(pControl));
            //log_information("Started!");
            status="started";
        }
    }
    void stopControlTask(){
        if(status!="stopping" && status!="stopped"){
            status = "stopping";
            pm.cancelAll();
            //log_information("Stopped!");
            status = "stopped";
        }
    }
    std::string getStatus(){
        return status;
    }
    Control *getControlledPointer(){
        return pControl;
    }
};


