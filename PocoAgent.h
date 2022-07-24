#pragma once
#include "Requester.h"
#include <Poco/Util/ServerApplication.h>


class PocoAgent: public Poco::Util::ServerApplication
{
private:
    Poco::SharedPtr<Requester> agentRequester;

    void defineOptions(Poco::Util::OptionSet& options);
public:
    PocoAgent();
    ~PocoAgent();
    int main(const std::vector<std::string>& args);
};


