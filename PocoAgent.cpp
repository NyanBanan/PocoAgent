#include "PocoAgent.h"

PocoAgent::PocoAgent(/* args */){
}

PocoAgent::~PocoAgent(){
}

int PocoAgent::main(const std::vector<std::string>& args){
    /*Poco::ClassLoader<Responcer> Loader;
    std::string lib ("Responcer");
    lib+=Poco::SharedLibrary::suffix();
    Loader.loadLibrary(lib);
    std::cout<<Loader.begin()->first;
    Poco::SharedPtr<Responcer>(Loader.create("Responcer"));*/
    Requester a(args[1]);
    system("pause");
    //Loader.unloadLibrary(lib);
    waitForTerminationRequest();
}

void PocoAgent::defineOptions(Poco::Util::OptionSet& options){
    Application::defineOptions(options);
    options.addOption(
        Poco::Util::Option("json-config", "jc", "Give the program path to the JSON config")
        .required(true)
        .repeatable(false));
}
