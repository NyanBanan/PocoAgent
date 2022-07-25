#include "PocoAgent.h"
#include "FunctionErrorsVariables.h"


std::string InGetParametersConfigNotOpen{"(In getParameters) Config file not open "};
std::string InGetParametersConfigError{"(In getParameters) Config file error "};
std::string InInitilizeLogger{"(In InitilizeLogger) "};
std::string InPingPong{"(In PingPong) "};
std::string InStartSession{"(In startSession) "};
std::string InBuildSender{"(In buildSender) "};
std::string InDestroySender{"(In destroySender) "};
std::string InStartPingPong{"(In startPingPong) "};
std::string InRunSender{"(In runSender) "};

PocoAgent::PocoAgent(/* args */){
}

PocoAgent::~PocoAgent(){
    logger.information("End of work");
    logger.close();
}

int PocoAgent::main(const std::vector<std::string>& args){
    initilizeLogger();
    startSession(*(getParameters(args[0])));
    pingPong();
    //startPingPong();
    waitForTerminationRequest();
}

void PocoAgent::defineOptions(Poco::Util::OptionSet& options){//задается обязательный параметр - путь к конфигу
    Application::defineOptions(options);
    options.addOption(
        Poco::Util::Option("json-config", "j", "Give the program path to the JSON config")
        .required(true)
        .repeatable(false));
}

void PocoAgent::initilizeLogger(){
    try{
        //file logger
        Poco::SimpleFileChannel::Ptr pFile(Poco::makeAuto<Poco::SimpleFileChannel>());
        pFile->setProperty("path", "Work.log");
        pFile->setProperty("rotation", "2 M");
        pFile->setProperty("flush", "true");
        //console logger
        Poco::ConsoleChannel::Ptr pCons(Poco::makeAuto<Poco::ConsoleChannel>());
        //synchronize loggers
        Poco::SplitterChannel::Ptr splitter_Channel(Poco::makeAuto<Poco::SplitterChannel>());
        splitter_Channel->addChannel(pFile);
        splitter_Channel->addChannel(pCons);
        //formatter
        Poco::PatternFormatter::Ptr patternFormatter(
            Poco::makeAuto<Poco::PatternFormatter>("[%Y-%m-%d  %H:%M] %p: %t"));
        patternFormatter->setProperty("times", "local");
        Poco::FormattingChannel::Ptr formattingChannel(
            Poco::makeAuto<Poco::FormattingChannel>(patternFormatter, splitter_Channel));
       
        Poco::Logger::root().setChannel(formattingChannel);
    }
    catch(Poco::Exception& e){
        Poco::Logger::root().error(InInitilizeLogger+e.name()+e.message());
    }
    catch(std::exception& e){
        Poco::Logger::root().error(InInitilizeLogger+e.what());
    }
}

void PocoAgent::startSession(const ConnectData& key_address){

    Poco::URI uri(key_address.address);
    path=uri.getPathAndQuery();
    //const Poco::Net::Context::Ptr context;

    try{
        if(path.empty())
            path="/";
        Poco::Net::initializeSSL();
        const Poco::Net::Context::Ptr context =Poco::makeAuto<Poco::Net::Context>(
            Poco::Net::Context::TLS_CLIENT_USE, "", "", "",
            Poco::Net::Context::VERIFY_NONE, 9, false,
            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        session = Poco::makeShared<Poco::Net::HTTPSClientSession>(uri.getHost(),uri.getPort(),context);
        req= Poco::makeShared<Poco::Net::HTTPRequest>(Poco::Net::HTTPRequest::HTTP_GET,path,Poco::Net::HTTPMessage::HTTP_1_1);
        req->setCredentials("Token",key_address.key);
        req->setContentType("application/json");
    }
    catch(Poco::Exception& e){
        logger.error(InStartSession+e.name()+e.message());
    }
    catch(std::exception& e){
        logger.error(InStartSession+e.what());
    }
}

Poco::SharedPtr<PocoAgent::ConnectData> PocoAgent::getParameters(const std::string& file_path){
    Poco::FileStream conf;
    Poco::File check(file_path);
    Poco::JSON::Object::Ptr pParam;
    Poco::JSON::Parser parser;

    try{
        if(check.exists()){
            conf.open(file_path, std::ios::in);
            parametersFile = file_path;
        }
        else 
            throw Poco::Exception("");
    }
    catch(Poco::FileException& e){
        logger.error(InGetParametersConfigNotOpen+e.name()+" "+e.message());
        return nullptr;
    }
    catch(std::exception& e){
        logger.error(InGetParametersConfigNotOpen+e.what());
        return nullptr;
    }

    try{
        pParam = parser.parse(conf).extract<Poco::JSON::Object::Ptr>();
    }   
    catch(Poco::Exception& e){
        logger.error(InGetParametersConfigError+e.name()+" "+e.message());
        conf.close();
        return nullptr;
    }
    catch(std::exception& e){
        logger.error(InGetParametersConfigError+e.what());
        conf.close();
        return nullptr;
    }
    std::string key=pParam->getValue<std::string>("key");
    std::string address=pParam->getValue<std::string>("https");
    return Poco::makeShared<PocoAgent::ConnectData>(key,address);
    
}

void PocoAgent::pingPong(){
    while(true){
        try{
            Poco::JSON::Parser parser;
            Poco::Dynamic::Var parsed_json;
            Poco::JSON::Object::Ptr pData;
            Poco::Net::HTTPResponse response;
            Poco::FileStream conf;

            Poco::ScopedLock lock(mutex);
            logger.information("Agent sends request");
            std::ostream& streq = session->sendRequest(*req);//открывается поток тела для GET, предпологаю, что можно обрабатывать входящие bool на сервере и реагировать на них

            streq << dllStartBuild<<" "<<dllEndBuild;//отправляю bool в теле GET
            
            logger.information("Agent sent request");

            std::istream &ret = session->receiveResponse(response);
            logger.information("Agent gets responce");

            parsed_json=parser.parse(ret);

            conf.open("Answer.json",std::ios::trunc);
    
            Poco::JSON::Stringifier::stringify(parsed_json,conf);

            //bool build =(parsed_json.extract<Poco::JSON::Object::Ptr>())->getValue<bool>("build");

            //if(build && !dllStartBuild && !dllEndBuild)
            //если от сервера поступил ответ с build:true и при этом сборка не начата и не закончена, то начни собирать и запускать Sender
                buildAndRunProcess();

            //if(!build && dllStartBuild && dllEndBuild)
            //если от сервера поступил ответ с build:false сборка начата и закончена, то начни уничтожать Sender
                destroyProcess();

            logger.information("Agent got responce");
            conf.close();
            Poco::Thread::yield();
        }
        catch(Poco::Exception& e){
            logger.error(InPingPong+e.name()+e.message());
        }
        catch(std::exception& e){
            logger.error(InPingPong+e.what());
        }
        Poco::Thread::sleep(10000);
    }
}

/* void PocoAgent::startPingPong(){
    try{
        Poco::RunnableAdapter<PocoAgent> runnable (*this,&PocoAgent::pingPong);
        Poco::Thread log_thread;
        log_thread.start(runnable);
    }
    catch(Poco::Exception& e){
        logger.error(InStartPingPong+e.name()+e.message());
    }
    catch(std::exception& e){
        logger.error(InStartPingPong+e.what());
    }   
} */

bool PocoAgent::buildSender(){
    try{
        std::string lib="libSender";
        lib+=Poco::SharedLibrary::suffix();

        loader.loadLibrary(lib);
        return true;
    }
    catch(Poco::Exception& e){
        logger.error(InBuildSender+e.name()+e.message());
        return false;
    }
    catch(std::exception& e){
        logger.error(InBuildSender+e.what());
        return false;
    }   
}

bool PocoAgent::runSender(){
    try{
        loader.instance("SenderImpl").sendData(session,parametersFile);
        return true;
    }
    catch(Poco::Exception& e){
        logger.error(InRunSender+e.name()+e.message());
        return false;
    }
    catch(std::exception& e){
        logger.error(InRunSender+e.what());
        return false;
    } 
}

void PocoAgent::buildAndRunProcess(){
    logger.warning("Sender start build");
    dllStartBuild=true;
    if(buildSender()){
        logger.warning("Sender successfully builded");
        dllEndBuild=true;
        if(runSender())
            logger.warning("Data successfully sended");
        else
            logger.error("Data send failed");
    }
    else {
        logger.error("build Sender failed");
        dllStartBuild=false;
    }
}

bool PocoAgent::destroySender(){
    try{
        std::string lib="libSender";
        lib+=Poco::SharedLibrary::suffix();
        loader.unloadLibrary(lib);
        return true;
    }
    catch(Poco::Exception& e){
        logger.error(InDestroySender+e.name()+e.message());
        return false;
    }
    catch(std::exception& e){
        logger.error(InDestroySender+e.what());
        return false;
    } 
}

void PocoAgent::destroyProcess(){
    logger.warning("Sender start destroy");
    dllStartBuild=false;
    if(destroySender()){
        logger.warning("Sender successfully destroyed");
        dllEndBuild=false;
    }
    else{
        logger.error("destroy Sender failed");
        dllStartBuild=true;
    }
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


