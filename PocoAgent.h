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
//for errors
#include "FunctionErrorsVariables.h"

#include "ISender.h"
#include <Poco/Util/ServerApplication.h>
#include <Poco/Environment.h>
#include <Poco/ClassLoader.h>

class PocoAgent: public Poco::Util::ServerApplication
{
public:
    
    struct ConnectData{
        std::string key;
        std::string address;
        ConnectData(const std::string& _key,const std::string& _address){
            key=_key;
            address=_address;
        }
    }; //Token to connect and URI adress
    
    PocoAgent();
    ~PocoAgent();
    int main(const std::vector<std::string>& args);
private:
    Poco::SharedPtr<Poco::Net::HTTPSClientSession> session;//Pointer to connect session so as not to create it at every request
    Poco::SharedPtr<Poco::Net::HTTPRequest> req; //Like session pointer, but for request
    std::string path; 
    Poco::Mutex mutex;
    Poco::Logger& logger=Poco::Logger::root();
    Poco::File parametersFile;
    Poco::ClassLoader<ISender> loader;
    std::string dllSenderName="libSender"+Poco::SharedLibrary::suffix();//для быстрого обращения к libSender.dll(.so)
    bool dllStartBuild=0;
    bool dllEndBuild=0;

    Poco::SharedPtr<ConnectData> getParameters(const std::string& config_path); //initilizer of Token and URI
    static void initilizeLogger(); //initilizer for logger
    void startSession(const ConnectData& key_address); 
    void pingPong();//каждые 10 сек обращается к серверу и просматривает нужно ли собирать или уничтожать Sender 
    //void startPingPong();//запускает pingPong в поток
    bool buildSender();//собирает Sender из dll
    bool runSender();//отправляет данные серверу
    void buildAndRunProcess();//отличается от последовательного вызова buildSender и runSender логгированием успешности работы и работой с булевыми сборки(dllStartBuild, dllEndBuild)
    bool destroySender();//уничтожает Sender
    void destroyProcess();//отличается от вызова destroySender логгированием успешности работы и работой с булевыми сборки(dllStartBuild, dllEndBuild)
    void defineOptions(Poco::Util::OptionSet& options);

};


