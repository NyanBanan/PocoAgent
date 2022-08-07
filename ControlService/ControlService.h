
#pragma once
#include "Poco/Util/ServerApplication.h"
#include <iostream>
#include "ControlController.h"
#include "../Logger.h"

class ControlService : public Poco::Util::ServerApplication{
protected:
    void initialize(Poco::Util::Application& self);
    void uninitialize();
    int main(const std::vector<std::string>& args);
};

