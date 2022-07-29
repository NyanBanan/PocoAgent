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
//for multi threading
#include <Poco/Thread.h>
#include <Poco/Mutex.h>
#include <Poco/RunnableAdapter.h>
//for logging
#include <Poco/SplitterChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/Logger.h>
#include <Poco/String.h>
#include <Poco/PatternFormatter.h>
//for connect
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/Context.h>
#include <Poco/SharedPtr.h>

#include "SenderPlugin/ISender.h"
#include <Poco/Util/ServerApplication.h>
#include <Poco/Environment.h>
#include <Poco/ClassLoader.h>

#include <Poco/UTF8Encoding.h>
#include "ServerTasks.h"

class PocoAgent: public Poco::Util::ServerApplication
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
        std::string available_interfaces;
        AgentParameters(const std::string& _address="https://api.gfias.com",const std::string& _user_name="username",const std::string& _password="password", 
                    const std::string& _name="None", const int& _area=0, const std::string& _key="", const int& _service_class=0,  const bool& _state=false,const bool& _status=false,
                    const std::string& _active_interface="None",const int& _uid=1,const std::string& _available_interfaces="None"):
                    address(_address),user_name(_user_name),password(_password),uid(_uid),name(_name),area(_area),service_class(_service_class),
                    key(_key),state(_state),status(_status),active_interface(_active_interface),available_interfaces(_available_interfaces){}
    }; //Token to connect and URI adress
    
    PocoAgent();
    ~PocoAgent();
    int main(const std::vector<std::string>& args);
protected:
    void initialize(Poco::Util::Application& self);
    void uninitialize();
private:
    Poco::TaskManager tm;
    Poco::AutoPtr<myServerTask> p_task;
    Poco::SharedPtr<Poco::Net::HTTPSClientSession> session;//Pointer to connect session so as not to create it at every request
    Poco::SharedPtr<AgentParameters> agent_param;
    std::string path; 
    Poco::Mutex mutex;
    Poco::Logger& logger=Poco::Logger::root();
    Poco::File parameters_file;

    void setAgentParameters(const std::string& file_path); //initilizer of Token and URI
    static void initilizeLogger(); //initilizer for logger
    void startSession(const std::string& address); 
    void pingPong();//каждые 10 сек обращается к серверу и просматривает нужно ли собирать или уничтожать Sender 

    //void defineOptions(Poco::Util::OptionSet& options);
    void getKeyFromService(const std::string& username,const std::string& password,std::string& string_to_write_key_here);
    void getAgentsData();
    void tryIdentifyAgent(const std::string& key,const int& area_id,const std::string& name);
    void createAgentOnServer();
    void ipRequest();
    void updateStatus(const bool& status);
    void updateState(const bool& state);
    void updateActiveInterfaces();
    void checkState();
};


