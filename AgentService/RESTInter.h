#pragma once
//std include
#include <iostream>
#include <fstream>
#include <sstream>
//for JSON
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Stringifier.h>
//for config and result
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>
#include <Poco/Net/NetSSL.h>
#include <Poco/Crypto/Crypto.h>

//for connect
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/Context.h>
#include <Poco/SharedPtr.h>
#include "../Plugins/AbstractPlugin.h"
#include "../Logger.h"

class RESTinter
{
public:
    struct AgentParameters{
        std::string address;
        std::string user_name;
        std::string password;
        int uid;
        std::string name;
        int area;
        int service_class;
        std::string key;
        bool state;
        bool status;
        std::string active_interface;
        AgentParameters(const std::string& _address="https://api.gfias.com",const std::string& _user_name="username",const std::string& _password="password",
                    const std::string& _name="None", const int& _area=0, const std::string& _key="", const int& _service_class=0,  const bool& _state=false,const bool& _status=false,
                    const std::string& _active_interface="None",const int& _uid=1):
                    address(_address),user_name(_user_name),password(_password),uid(_uid),name(_name),area(_area),service_class(_service_class),
                    key(_key),state(_state),status(_status),active_interface(_active_interface){}
    };
    


    RESTinter(AbstractPlugin* _plugin);
    ~RESTinter();
    void getKeyFromService(const std::string& username,const std::string& password,std::string& string_to_write_key_here);
    //const Poco::SharedPtr<PocoAgent::AgentParameters> getAgentParam();
    void tryIdentifyAgent(const std::string& key,const int& area_id,const std::string& name);
    void createAgentOnServer();
    int ipRequest();
    void updateStatus(const bool& status);
    void updateState(const bool& state);
    void updateActiveInterfaces();
    void checkState();
    /*bool getState();
    bool getStatus();
    void setState(const bool& _state);
    void setStatus(const bool& _status);*/
    void pingPong();
    void repairRegistration();
    void setAgentParameters(const std::string& file_path); //initilizer of Token and URI
    void startSession(const std::string& address);
    void startPlugin();
    void stopPlugin();
private:
    Poco::SharedPtr<Poco::Net::HTTPSClientSession> session;//Pointer to connect session so as not to create it at every request
    Poco::SharedPtr<AgentParameters> agent_param;
    AbstractPlugin* plugin;
    Poco::Mutex mutex;
};



