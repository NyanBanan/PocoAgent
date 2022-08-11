#pragma once

#include <Poco/Util/ServerApplication.h>
#include <Poco/Environment.h>
#include "ServerTasks.h"

class PocoAgent: public Poco::Util::ServerApplication
{
protected:
    void initialize(Poco::Util::Application& self);
    void uninitialize();
    int main(const std::vector<std::string>& args);
};


