#include "ServerTasks.h"

myServerTask::myServerTask(RESTinter* _rest):Poco::Task("MainBody"){
	rest=_rest;
}

myServerTask::~myServerTask(){

}

void myServerTask::runTask(){
	rest->setAgentParameters("C:/ngids/parameters.json");
        while (!isCancelled()){
            rest->pingPong();
            sleep(5000);
        }

}


