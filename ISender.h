#pragma once
//for JSON
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Stringifier.h>
//for config and result
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>
#include <Poco/SharedPtr.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>

class ISender
{
private:
    /* data */
public:
    ISender(/* args */);
    virtual ~ISender();
    virtual void sendData(Poco::SharedPtr<Poco::Net::HTTPSClientSession> session, const Poco::File& param_file)=0;
};