#include "ServerTasks.h"

myServerTask::myServerTask():Poco::Task("GetInterfaces"){ //Регистрируем задачу под именем "GetInterfaces"
	libName = "libSender";
	libName += Poco::SharedLibrary::suffix(); // append .dll or .so
	loader.loadLibrary(libName);
    available_interfaces="None";
}

myServerTask::~myServerTask(){
	loader.classFor("SenderPlugin").autoDelete(pPlugin);
	loader.unloadLibrary(libName);
}

void myServerTask::runTask()
{
    pPlugin = loader.create("SenderPlugin");
	Poco::Util::Application& app = Poco::Util::Application::instance();
	std::string temp;
	while (!isCancelled()){
			log_information("Service is alive " + Poco::DateTimeFormatter::format(app.uptime()));
			temp = pPlugin->routine();
			available_interfaces=temp;
			log_information(temp);
			Poco::Thread::sleep(SCAN_INTERVAL);
	}
}

std::string myServerTask::getAvailableInterfaces(){
	return available_interfaces;
}

void myServerTask::clear(){
	available_interfaces="None";
}