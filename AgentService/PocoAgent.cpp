#include "PocoAgent.h"

void PocoAgent::initialize(Poco::Util::Application& self){
	//Загружаем конфигурацию
	loadConfiguration();
		
	//Инициализируем ServerApplication
	Poco::Util::ServerApplication::initialize(self);

    init_logger();
}

void PocoAgent::uninitialize()
{
	log_information("Shutting down");
	Poco::Util::ServerApplication::uninitialize();
}

int PocoAgent::main(const std::vector<std::string>& args){
    Poco::TaskManager tm;
    {
        LibraryLoader lib;
        RESTinter rest(lib.getInstanse());

        tm.start(new myServerTask(&rest));

        waitForTerminationRequest();
        tm.cancelAll();
    }//контроль времени жизни RESTinter
	tm.joinAll();
    return Poco::Util::Application::EXIT_OK;
}

/* void PocoAgent::defineOptions(Poco::Util::OptionSet& options){//задается обязательный параметр - путь к конфигу
    Application::defineOptions(options);
    options.addOption(
        Poco::Util::Option("json-config", "j", "Give the program path to the JSON config")
        .required(true)
        .repeatable(false));
} */






/*Poco::TaskManager::TaskList tmlist=tm.taskList();
    auto finded=std::find_if(tmlist.begin(),tmlist.end(), [&](const Poco::TaskManager::TaskPtr x){
        return x->name()=="GetInterfaces";
    });
    if(tmlist.end()!=finded)
        p_task=(*finded).cast<myServerTask>().get();*/

