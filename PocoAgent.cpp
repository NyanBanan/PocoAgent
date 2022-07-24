#include "PocoAgent.h"

PocoAgent::PocoAgent(/* args */){
    agentRequester = Poco::makeShared<Requester>();
}

PocoAgent::~PocoAgent(){
}

int PocoAgent::main(const std::vector<std::string>& args){
    agentRequester->initilizeLogger();
    agentRequester->logger.information(args[0]);
    agentRequester->startSession(*(agentRequester->getParameters(args[0])));

    Poco::RunnableAdapter<Requester> runnable (*agentRequester, &Requester::logging);
    
    Poco::Thread log_thread;
    log_thread.start(runnable);
    agentRequester->writeJsonAnswer();
    system("pause");
    
    waitForTerminationRequest();
}

void PocoAgent::defineOptions(Poco::Util::OptionSet& options){
    Application::defineOptions(options);
    options.addOption(
        Poco::Util::Option("json-config", "j", "Give the program path to the JSON config")
        .required(true)
        .repeatable(false));
}

//POCO_SERVER_MAIN(PocoAgent);
int wmain(int argc, wchar_t** argv) {
    try { 
        PocoAgent app; 
        return app.run(argc,argv);
        }
    catch (Poco::Exception& exc) {
        std::cerr << exc.displayText() << std::endl;
        return Poco::Util::Application::EXIT_SOFTWARE;
        } 
}
/*int wmain(const std::vector<std::string>& args){
    PocoAgent pa;
    std::vector<std::string> a;
    a.push_back("parameters.json");
    return pa.run(a);
}*/
