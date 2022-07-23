#pragma once
#include "Poco/ClassLoader.h"
#include "Poco/Manifest.h"
#include "Requester.h"
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Application.h>
class PocoAgent: public Poco::Util::ServerApplication
{
private:
    Poco::SharedPtr<Requester> agentRequester;

    void defineOptions(Poco::Util::OptionSet& options);
public:
    PocoAgent(/* args */);
    ~PocoAgent();
    int main(const std::vector<std::string>& args);
};


