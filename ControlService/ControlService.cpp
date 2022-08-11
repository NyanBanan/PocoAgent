
#include "ControlService.h"
#include <iostream>
void ControlService::initialize(Poco::Util::Application& self){
    //Загружаем конфигурацию
    loadConfiguration();

    //Инициализируем ServerApplication
    Poco::Util::ServerApplication::initialize(self);

}

void ControlService::uninitialize()
{
    Poco::Util::ServerApplication::uninitialize();
}

int ControlService::main(const std::vector<std::string>& args){
    {
        ControlTaskController control(new Control);
        //log_information(control.getControlledPointer()->decodeWriteHash(control.getControlledPointer()->handWriteHash("PocoAgent001")));
        control.startControlTask();
        waitForTerminationRequest();
    }
    return Poco::Util::Application::EXIT_OK;
}