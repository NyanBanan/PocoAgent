#ifndef AbstractPlugin_INCLUDED
#define AbstractPlugin_INCLUDED

#include <iostream>
#include "Poco/Task.h"
class AbstractPlugin
{
protected:
    class PluginTask: public Poco::Task
    {
    public:
        explicit PluginTask(const std::string& name):Task(name){}
        virtual ~PluginTask(){}
        virtual void runTask()=0;
    };

    PluginTask * int_task;
public:
    AbstractPlugin();
    virtual ~AbstractPlugin();
    virtual std::string getStatus() = 0;
    virtual std::string getData()=0;
    virtual void start() = 0;
    virtual void stop() = 0;
};
#endif // AbstractPlugin.h