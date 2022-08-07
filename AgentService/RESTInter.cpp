#include "RESTInter.h"
RESTinter::RESTinter(ISender* plugin){
    pm = Poco::makeShared<PluginTaskController>(plugin);
}
RESTinter::~RESTinter(){
    pm->stopPlugin();
    updateStatus(0);
    updateState(0);
}

void RESTinter::startSession(const std::string& address){
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
        log_error(e.name());
    }
    catch(std::exception& e){
        log_error(" In startSession ");
        log_error(e.what());
    }
}

void RESTinter::setAgentParameters(const std::string& file_path){
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
            if(check.createFile()) {
                conf.open(file_path, std::ios::out);
                conf << "{\"api_url\":\"https://api.gfias.com\",\n"
                     << "\"username\":\"\",\n"
                     << "\"password\":\"\",\n"
                     << "\"name\":\"LocalNetAgent\",\n"
                     << "\"area\":\"1\",\n"
                     << "\"service_class\":\"0\",\n"
                     << "\"key\":\"\"\n}";
                conf.close();
                log_warning(" Config file didn't found, new config file created please fill them");
            }
            else
                log_warning("Config file path incorrect");
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
        if(key.empty())
            getKeyFromService(username,password,key);
        agent_param = Poco::makeShared<RESTinter::AgentParameters>(address,username,password,name,area,key);
        tryIdentifyAgent(key,area,name);
        return;
    }   
    catch(Poco::Exception& e){
        log_error(" In set agent parameters ");
        log_error(e.name());
        return;
    }
    catch(std::exception& e){
        log_error(" In set agent parameters ");
        log_error(e.what());
        return;
    }
    }
    

void RESTinter::getKeyFromService(const std::string& username,const std::string& password, std::string& string_to_write_key_here){
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
            log_information("Agent successfully authorized");
        }
        else{
            log_error("Authorization error, check username and password");
        }
            log_information(std::to_string(response.getStatus())+" "+response.getReason());
    }
    catch(Poco::Exception& e){
        log_error("In get key from service, please, check your internet connection ");
        log_error(e.name());
    }
    catch(std::exception& e){
        log_error("In get key from service, please, check your internet connection ");
        log_error(e.what());
    }
}

void RESTinter::tryIdentifyAgent(const std::string& key,const int& area_id,const std::string& name){
    bool find=false;
    Poco::JSON::Object::Ptr tempObj;
    Poco::Net::HTTPResponse response;
    Poco::JSON::Parser parser;
    std::string path("/ids/api/service/list/?area_id="+std::to_string(area_id));
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,path,Poco::Net::HTTPMessage::HTTP_1_1);
    req.setCredentials("Token",key);
    req.setContentType("application/json");
    try {
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
                    ipRequest();
                    updateStatus("0");
                    updateStatus("1");
                    updateState("0");
                    updateState("1");
                    pm->startPlugin();
                    log_information("The state has been sent to the server for autorun");
                    break;
                }
            }
            if(!find){
                log_warning("Agent didn`t found");
                createAgentOnServer();
            }
        }
        else {
            log_error("Agent authorization error");
            log_error(std::to_string(response.getStatus())+" "+response.getReason());
        }
    }
    catch(Poco::Exception& e){
        log_error("In trying identify agent, please, check your internet connection ");
        log_error(e.name());
        return;
    }
    catch(std::exception& e){
        log_error("In trying identify agent, please, check your internet connection ");
        log_error(e.what());
        return;
    }
}

int RESTinter::ipRequest(){
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
        return response.getStatus();
    }
    catch(Poco::Exception& e){
        log_error("In ip request, please, check your internet connection ");
        log_error(e.name());
        return 500;
    }
    catch(std::exception& e){
        log_error("In ip request, please, check your internet connection ");
        log_error(e.what());
        return 500;
    }
}

void RESTinter::updateStatus(const bool& status){

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
        log_error(e.name());
    }
    catch(std::exception& e){
        log_error(" In update status ");
        log_error(e.what());
    }
}


void RESTinter::updateState(const bool& state){  

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
        log_error(e.name());
    }
    catch(std::exception& e){
        log_error(" In update state ");
        log_error(e.what());
    }
}


void RESTinter::createAgentOnServer(){
    log_information("Agent creating on server");
    Poco::Net::HTTPResponse response;
    Poco::JSON::Parser parser;

    std::string path("/ids/api/service/create/");
    std::string body("{\"name\":\""+agent_param->name+"\","
            +"\"status\":\""+std::to_string(agent_param->status)+"\","
            +"\"state\":\""+std::to_string(agent_param->state)+"\","
            +"\"active_interface\":\""+agent_param->active_interface+"\","
            +"\"avaliable_interfaces\":\""+pm->getPluginInterfacesString()+"\","//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
        log_error(e.name());
    }
    catch(std::exception& e){
        log_error(" In agent create ");
        log_error(e.what());
    }
}

void RESTinter::checkState(){
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
        log_error(e.name());
    }
    catch(std::exception& e){
        log_error(" In receive status ");
        log_error(e.what());
    }
}

void RESTinter::updateActiveInterfaces(){
    Poco::Net::HTTPResponse response;
    Poco::JSON::Parser parser;
    std::string body("{\"available_interfaces\":"+pm->getPluginInterfacesString()+"}");//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    std::string path ("/ids/api/service/update/available_interfaces/"+std::to_string(agent_param->uid)+"/");
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST,path,Poco::Net::HTTPMessage::HTTP_1_1);
    req.setCredentials("Token",agent_param->key);
    req.setContentType("application/json");
    req.setContentLength(body.length());
    try{
        session->sendRequest(req)<<body;
        session->receiveResponse(response);
        log_information("The available interfaces have been sent to the server");
            //log_information(body);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        log_information(std::to_string(response.getStatus())+" "+response.getReason());
    }
    catch(Poco::Exception& e){
        log_error(" In update interfaces ");
        log_error(e.name());
    }
    catch(std::exception& e){
        log_error(" In update interfaces ");
        log_error(e.what());
    }
}

void RESTinter::pingPong(){
    if(agent_param->key.empty()){
        if (agent_param->name.empty() || agent_param->user_name.empty() || agent_param->password.empty()) {
            log_information("Name, password and user_name fields must be filled in parameters file");
            return;
        }
        else {
            getKeyFromService(agent_param->user_name, agent_param->password, agent_param->key);
            tryIdentifyAgent(agent_param->key,agent_param->area,agent_param->name);
        }
    }

    if(ipRequest()==200) {
        if(!agent_param->status)
            log_information("Agent status 0, waiting for commands from the server...");
        checkState();
        updateStatus(agent_param->status);
        if (agent_param->status)
            updateActiveInterfaces();

        if (agent_param->state && !agent_param->status) {
            pm->startPlugin();
            agent_param->status = true;
        }
        else if (!agent_param->state && agent_param->status) {
            pm->stopPlugin();
            agent_param->status = false;
        }
    }
    else
        log_error("Connect to REST error");
}