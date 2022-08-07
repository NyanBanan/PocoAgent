#pragma once
//for config and result
#include "../Logger.h"

class ISender
{
private:
    std::string str_interfaces;
public:
    ISender();
    virtual ~ISender();
    virtual std::string routine() = 0;
    virtual std::string getInterfacesString() = 0;
    virtual void clear()=0;
};