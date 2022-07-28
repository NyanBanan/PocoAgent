#include "ServerTasks.h"

myServerTask::myServerTask():Poco::Task("GetInterfaces"){ //Регистрируем задачу под именем "GetInterfaces"
	libName = "libSender";
	libName += Poco::SharedLibrary::suffix(); // append .dll or .so
	loader.loadLibrary(libName);
}

myServerTask::~myServerTask(){
	loader.classFor("SenderPlugin").autoDelete(pPlugin);
	loader.unloadLibrary(libName);
}

void myServerTask::runTask()
{
    pPlugin = loader.create("SenderPlugin");
	Poco::Util::Application& app = Poco::Util::Application::instance();
	while (!isCancelled()){
			log_information("Service is alive " + Poco::DateTimeFormatter::format(app.uptime()));
			log_information(pPlugin->routine());
			Poco::Thread::sleep(SCAN_INTERVAL);
	}
}

void myServerTask::cancel(){
    loader.destroy("SenderPlugin",pPlugin);
    Poco::Task::cancel();
}
    