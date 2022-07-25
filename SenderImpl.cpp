
#include "ISender.h"
#include <Poco/ClassLibrary.h>
#include <Poco/Logger.h>
#include <iostream>
class SenderImpl:public ISender
{
private:
    /* data */
public:
    SenderImpl(/* args */);
    ~SenderImpl();
    void sendData(Poco::SharedPtr<Poco::Net::HTTPSClientSession> session, const Poco::File& param_file);
};

SenderImpl::SenderImpl(/* args */)
{
}

SenderImpl::~SenderImpl()
{
}

void SenderImpl::sendData(Poco::SharedPtr<Poco::Net::HTTPSClientSession> session, const Poco::File& param_file){
    Poco::FileStream conf;
    Poco::JSON::Object::Ptr pData;
    Poco::JSON::Parser parser;
    Poco::SharedPtr<Poco::Net::HTTPRequest> postReq;
    std::string key;
    std::string path;
    Poco::URI uri;
    
    conf.open(param_file.path(), std::ios::in);
    pData = parser.parse(conf).extract<Poco::JSON::Object::Ptr>();

    key=pData->getValue<std::string>("key");
    uri = pData->getValue<std::string>("https");
    path=uri.getPathAndQuery();

    postReq = Poco::makeShared<Poco::Net::HTTPRequest>(Poco::Net::HTTPRequest::HTTP_POST,path,Poco::Net::HTTPMessage::HTTP_1_1);
    postReq->setCredentials("Token",key);
    postReq->setContentType("application/json");

    std::ostream& sended=session->sendRequest(*postReq);

    sended<<"{username:"<<pData->getValue<std::string>("username")<<","
        <<"uid:"<<pData->getValue<std::string>("uid")<<","
        <<"name:"<<pData->getValue<std::string>("name")<<","
        <<"area:"<<pData->getValue<std::string>("area")<<","
        <<"service_class:"<<pData->getValue<std::string>("service_class")<<"}";

    Poco::Logger::root().information("Im testing dll");
}

POCO_BEGIN_MANIFEST(ISender)
POCO_EXPORT_SINGLETON(SenderImpl)
POCO_END_MANIFEST
 