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
            /*rest->checkRegistration();
            if(rest->ipRequest()==200) {
                if(!rest->getStatus())
                    log_information("Agent status 0, waiting for commands from the server...");
                rest->checkState();
                rest->updateStatus(rest->getStatus());
                if (rest->getStatus())
                    rest->updateActiveInterfaces();

                if (rest->getState() && !rest->getStatus()) {
                    rest->plugin->start();
                    rest->setStatus(true);
                }
                else if (!rest->getState() && rest->getStatus()) {
                    plugin->stop();
                    rest->setStatus(false);
                }
            }
            else
                log_error("Connect to REST error");*/
            sleep(5000);
        }

}


