
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
    void testdll();
};

SenderImpl::SenderImpl(/* args */)
{
}

SenderImpl::~SenderImpl()
{
}

void SenderImpl::testdll(){
    Poco::Logger::root().information("Im testing dll");
}

POCO_BEGIN_MANIFEST(ISender)
POCO_EXPORT_SINGLETON(SenderImpl)
POCO_END_MANIFEST
 
void pocoInitializeLibrary() {
    std::cout << "Library initializing" << std::endl;
}
 
void pocoUninitializeLibrary() {
    std::cout << "Library uninitializing" << std::endl;
}