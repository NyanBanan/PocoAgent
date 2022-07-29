#include "PocoAgent.h"

PocoAgent::PocoAgent(/* args */){
}

PocoAgent::~PocoAgent(){
}

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
    if(Poco::Environment::isWindows())
        setlocale(LC_ALL, "Russian");

    Poco::TaskManager tm;
    p_task=new myServerTask;
    tm.start(p_task);
    
    setAgentParameters("C:/ngids/parameters.json");
    //pingPong();
    Poco::RunnableAdapter<PocoAgent> ra(*this, &PocoAgent::pingPong);
    ra.run();
        
    
    waitForTerminationRequest();
    tm.cancelAll();
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

void PocoAgent::startSession(const std::string& address){
    Poco::URI uri(address);
    Poco::Net::Context::Ptr context;

    try{
        Poco::Net::initializeSSL();
        context = Poco::makeAuto<Poco::Net::Context>(
            Poco::Net::Context::TLS_CLIENT_USE, "", "", "",
            Poco::Net::Context::VERIFY_NONE, 9, false,
            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        session = Poco::makeShared<Poco::Net::HTTPSClientSession>(uri.getHost(),uri.getPort(),context);
        log_information("Session started");
    }
    catch(Poco::Exception& e){
        log_error(" In startSession ");
        log_error(e.message());
    }
    catch(std::exception& e){
        log_error(" In startSession ");
        log_error(e.what());
    }
}

void PocoAgent::setAgentParameters(const std::string& file_path){
    Poco::FileStream conf;
    Poco::File check(file_path);
    Poco::JSON::Object::Ptr pParam;
    Poco::JSON::Parser parser;
    std::string key;
    std::string address;
    std::string username;
    std::string password;
    std::string name;
    int area;

    try{
        if(check.exists()){
            conf.open(file_path, std::ios::in);
        }
        else{
            check.createFile();
            conf.open(file_path,std::ios::out);
            conf<<"{\"api_url\":\"https://api.gfias.com\",\n"
            <<"\"username\":\"\",\n"
            <<"\"password\":\"\",\n"
            <<"\"uid\":\"333\",\n"
            <<"\"name\":\"LocalNetAgent\",\n"
            <<"\"area\":\"1\",\n"
            <<"\"service_class\":\"0\",\n"
            <<"\"key\":\"c55a1da1bff5a2ad82124aac5332f9e86385cd25\"\n}";
            conf.close();
            log_warning(" Config file didn't found, new config file created ");
            return;
        }
    
        pParam = parser.parse(conf).extract<Poco::JSON::Object::Ptr>();

        address=pParam->getValue<std::string>("api_url");
        startSession(address);
        username=pParam->getValue<std::string>("username");
        password=pParam->getValue<std::string>("password");
        key=pParam->getValue<std::string>("key");
        name=pParam->getValue<std::string>("name");
        area=pParam->getValue<int>("area_id");
        if(key=="")
            getKeyFromService(username,password,key);
        agent_param = Poco::makeShared<PocoAgent::AgentParameters>(address,username,password,name,area,key);
        tryIdentifyAgent(key,area,name);
    }   
    catch(Poco::Exception& e){
        log_error(" In set agent parameters ");
        log_error(e.message());
        return;
    }
    catch(std::exception& e){
        log_error(" In set agent parameters ");
        log_error(e.what());
        return;
    }
    }
    

void PocoAgent::getKeyFromService(const std::string& username,const std::string& password, std::string& string_to_write_key_here){
    try{
        Poco::Net::HTTPResponse response;
        Poco::JSON::Parser parser;
        std::string body("{\"username\":\""+username+"\",\"password\":\""+password+"\"}");
        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST,"/ids/api/auth/",Poco::Net::HTTPMessage::HTTP_1_1); 
        req.setContentLength(body.length());
        req.setContentType("application/json");
        session->sendRequest(req)<<body;
        std::istream& sres = session->receiveResponse(response);
        if (response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK){
            string_to_write_key_here = parser.parse(sres).extract<Poco::JSON::Object::Ptr>()->getValue<std::string>("token");
            log_information("Agent successesfully authorized");
        }
        else{
            log_error("Authorization error");
        }
            log_information(std::to_string(response.getStatus())+" "+response.getReason());
    }
    catch(Poco::Exception& e){
        log_error(" In get key from service ");
        log_error(e.name());
    }
    catch(std::exception& e){
        log_error(" In get key from service ");
        log_error(e.what());
    }
}

void PocoAgent::tryIdentifyAgent(const std::string& key,const int& area_id,const std::string& name){
    bool find=false;
    Poco::JSON::Object::Ptr tempObj;
    Poco::Net::HTTPResponse response;
    Poco::JSON::Parser parser;
    std::string path("/ids/api/service/list/?area_id="+std::to_string(area_id));
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,path,Poco::Net::HTTPMessage::HTTP_1_1);
    req.setCredentials("Token",key);
    req.setContentType("application/json");
    session->sendRequest(req);
    std::istream& sres = session->receiveResponse(response);
    if (response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK){
        log_information("Agents data successesfuly received");
        auto array=parser.parse(sres).extract<Poco::JSON::Array::Ptr>();
        auto it = array->begin();
        auto end = array->end();
        for (;it!=end;++it){
            tempObj=it->extract<Poco::JSON::Object::Ptr>();
            if(tempObj->getValue<std::string>("name")==name){
                log_information("Agent identified, responced active state and interface");
                agent_param->uid=tempObj->getValue<int>("id");
                agent_param->state=true;
                agent_param->status=true;
                log_information("Assigned agent ID: " + std::to_string(agent_param->uid));
                agent_param->active_interface=tempObj->getValue<std::string>("active_interface");
                log_information("Current interface of the agent environment: " + agent_param->active_interface);
                find=true;
                try{
                    ipRequest();
                    updateStatus("0");
                    updateStatus("1");
                    updateState("0");
                    updateState("1");
                    log_information("The state has been sent to the server for autorun");
                }
                catch(Poco::Exception& e){
                    log_error(" In trying identify agent ");
                    log_error(e.message());
                    return;
                }
                catch(std::exception& e){
                    log_error(" In trying identify agent ");
                    log_error(e.what());
                    return;
                }
                break;
                }
            } 
        if(!find){
           log_warning("Agent didn`t found");
           createAgentOnServer();
        }
        }
    else
        log_error("Authorization error");
    }

void PocoAgent::ipRequest(){
    try{
        Poco::Net::HTTPResponse response;
        Poco::JSON::Parser parser;
        std::string body("{\"service\":"+std::to_string(agent_param->uid)+"}");
        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST,"/ids/api/ip",Poco::Net::HTTPMessage::HTTP_1_1);
        req.setCredentials("Token",agent_param->key);
        req.setContentLength(body.length());
        req.setContentType("application/json");
        //Poco::File f("test.json");
        //f.createFile();
        //Poco::FileStream a(f.path(),std::ios::out);
        session->sendRequest(req)<<body;
        //a.close();
        session->receiveResponse(response);
        log_information("An ip request was made to the server");
        log_information(std::to_string(response.getStatus())+" "+response.getReason());
    }
    catch(Poco::Exception& e){
        log_error(" In ip request ");
        log_error(e.message());
    }
    catch(std::exception& e){
        log_error(" In ip request ");
        log_error(e.what());
    }
}

void PocoAgent::updateStatus(const bool& status){
    try{
        Poco::Net::HTTPResponse response;
        Poco::JSON::Parser parser;
        std::string body("{\"status\":"+std::to_string(status)+"}");
        std::string path("/ids/api/service/update/status/"+std::to_string(agent_param->uid)+"/");
        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST,path,Poco::Net::HTTPMessage::HTTP_1_1);
        req.setCredentials("Token",agent_param->key);
        req.setContentLength(body.length());
        req.setContentType("application/json");
        session->sendRequest(req)<<body;
        session->receiveResponse(response);
        if (response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK){
            log_information("The status has been sent to the server ");
        }
        else{ 
            log_error("Update status error");
        }
            log_information(std::to_string(response.getStatus())+" "+response.getReason());
    }
    catch(Poco::Exception& e){
        log_error(" In update status ");
        log_error(e.message());
    }
    catch(std::exception& e){
        log_error(" In update status ");
        log_error(e.what());
    }
}

void PocoAgent::updateState(const bool& state){  
    try{
        Poco::Net::HTTPResponse response;
        Poco::JSON::Parser parser;
        std::string body("{\"state\":"+std::to_string(state)+"}");
        std::string path("/ids/api/service/update/state/"+std::to_string(agent_param->uid)+"/");
        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST,path,Poco::Net::HTTPMessage::HTTP_1_1);
        req.setContentLength(body.length());
        req.setCredentials("Token",agent_param->key);
        req.setContentType("application/json");
        session->sendRequest(req)<<body;
        session->receiveResponse(response);
        if (response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK){
            log_information("The state has been sent to the server");
        }
        else{
            log_error("Update state error");
        }   
            log_information(std::to_string(response.getStatus())+" "+response.getReason());
    }
    catch(Poco::Exception& e){
        log_error(" In update state ");
        log_error(e.message());
    }
    catch(std::exception& e){
        log_error(" In update state ");
        log_error(e.what());
    }
}

void PocoAgent::createAgentOnServer(){
    log_information("Agent creating on server");
    Poco::Net::HTTPResponse response;
    Poco::JSON::Parser parser;
    std::string path("/ids/api/service/create/");
    std::string body("{\"name\":\""+agent_param->name+"\","
            +"\"status\":\""+std::to_string(agent_param->status)+"\","
            +"\"state\":\""+std::to_string(agent_param->state)+"\","
            +"\"active_interface\":\""+agent_param->active_interface+"\","
            +"\"avaliable_interfaces\":\""+p_task->getAvailableInterfaces()+"\","//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            +"\"ip_address\":\""+"127.0.0.1"+"\","
            +"\"service_class\":"+std::to_string(agent_param->service_class)+","
            +"\"key\":\""+agent_param->key+"\","
            +"\"area\":"+std::to_string(agent_param->area)+","
            +"\"params\":"+"[\"1\",\"1\"]"+"}");
            Poco::File f("test.json");
        f.createFile();
        Poco::FileStream a(f.path(),std::ios::out);
        a<<body;
        std::cout<<body;
        a.close();
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST,path,Poco::Net::HTTPMessage::HTTP_1_1);
    req.setContentLength(body.length());
    req.setCredentials("Token",agent_param->key);
    req.setContentType("application/json");
    try{
        session->sendRequest(req)<<body;
        std::istream& sres = session->receiveResponse(response);
        if (response.getStatus()==Poco::Net::HTTPResponse::HTTP_CREATED){
            log_information(" Agent create successful");
            auto obj = parser.parse(sres).extract<Poco::JSON::Object::Ptr>();
            agent_param->uid=obj->getValue<int>("id");
            log_information("The new agent has been assigned an ID:"+std::to_string(agent_param->uid));
        }
        else{
            log_error("Create agent error");
            log_error(std::to_string(response.getStatus())+" "+response.getReason());
        }
    }
    catch(Poco::Exception& e){
        log_error(" In agent create ");
        log_error(e.message());
    }
    catch(std::exception& e){
        log_error(" In agent create ");
        log_error(e.what());
    }
}

void PocoAgent::checkState(){
    Poco::Net::HTTPResponse response;
    Poco::JSON::Parser parser;
    std::string path ("/ids/api/service/list/"+std::to_string(agent_param->uid)+"/");
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,path,Poco::Net::HTTPMessage::HTTP_1_1);
    req.setCredentials("Token",agent_param->key);
    req.setContentType("application/json");
    try{
        session->sendRequest(req);
        std::istream& sres=session->receiveResponse(response);
        if(response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK){
            log_information("A state request has been made to the server");
            log_information(std::to_string(response.getStatus())+" "+response.getReason());
            auto obj = parser.parse(sres).extract<Poco::JSON::Object::Ptr>();
            agent_param->state=obj->getValue<bool>("state");
            log_information("Agent state:"+std::to_string(agent_param->state));
            agent_param->active_interface=obj->getValue<std::string>("active_interface");
            log_information("Active interface of the agent enviroment:"+agent_param->active_interface);
        }
        else{
            log_error("Receive status error");
        }
            log_information(std::to_string(response.getStatus())+" "+response.getReason());
    }
    catch(Poco::Exception& e){
        log_error(" In receive status ");
        log_error(e.message());
    }
    catch(std::exception& e){
        log_error(" In receive status ");
        log_error(e.what());
    }
}

void PocoAgent::updateActiveInterfaces(){
    Poco::Net::HTTPResponse response;
    Poco::JSON::Parser parser;
    std::string body("{\"available_interfaces\":"+p_task->getAvailableInterfaces()+"}");//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    std::string path ("/ids/api/service/update/available_interfaces/"+std::to_string(agent_param->uid)+"/");
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST,path,Poco::Net::HTTPMessage::HTTP_1_1);
    req.setCredentials("Token",agent_param->key);
    req.setContentType("application/json");
    req.setContentLength(body.length());
    try{
        session->sendRequest(req)<<body;
        session->receiveResponse(response);
        log_information("The available interfaces have been sent to the server");
        log_information(body);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        log_information(std::to_string(response.getStatus())+" "+response.getReason());
    }
    catch(Poco::Exception& e){
        log_error(" In update interfaces ");
        log_error(e.message());
    }
    catch(std::exception& e){
        log_error(" In update interfaces ");
        log_error(e.what());
    }
}

void PocoAgent::startGetInterfaces(){
    if(p_task->state()!=Poco::Task::TASK_RUNNING && p_task->state()!=Poco::Task::TASK_STARTING){
        p_task->run();
        while(p_task->state()==Poco::Task::TASK_STARTING){
            log_information("Plugin starting");
            Poco::Thread::sleep(5000);
        }
        agent_param->status=true;
    }
}

void PocoAgent::stopGetInterfaces(){
    if(p_task->state()!=Poco::Task::TASK_CANCELLING && p_task->state()!=Poco::Task::TASK_FINISHED){
        p_task->clear();
        p_task->cancel();
        while(p_task->state()==Poco::Task::TASK_CANCELLING){
            log_information("Plugin stopping");
            Poco::Thread::sleep(3000);
        }
        agent_param->status=false;
    }
}

void PocoAgent::pingPong(){
    while(true){
        if(this->agent_param->status==0){
            log_information("Agent status 0, waiting for commands from the server...");
        }
        this->ipRequest();
        this->checkState();
        this->updateStatus(agent_param->status);
        this->updateActiveInterfaces();

        if(this->agent_param->state==true)//...............................................................................
            this->startGetInterfaces();
        else if(this->agent_param->state==false)
            this->stopGetInterfaces();
        Poco::Thread::sleep(5000);
    }
}

POCO_SERVER_MAIN(PocoAgent);


/*Poco::TaskManager::TaskList tmlist=tm.taskList();
    auto finded=std::find_if(tmlist.begin(),tmlist.end(), [&](const Poco::TaskManager::TaskPtr x){
        return x->name()=="GetInterfaces";
    });
    if(tmlist.end()!=finded)
        p_task=(*finded).cast<myServerTask>().get();*/

