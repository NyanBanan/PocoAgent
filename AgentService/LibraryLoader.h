#pragma once
#include "Poco/ClassLoader.h"
#include "Poco/Manifest.h"
#include "../SenderPlugin/ISender.h"
#include <iostream>
#include "../Logger.h"

typedef Poco::ClassLoader<ISender> PluginLoader;
typedef Poco::Manifest<ISender> PluginManifest;

class LibraryLoader
{
private:
    PluginLoader loader;
	ISender* pPlugin=nullptr;
	std::string libName;
public:
    LibraryLoader(){
        libName = "libSender";
        libName += Poco::SharedLibrary::suffix(); // append .dll or .so
        try {
            loader.loadLibrary(libName);
            pPlugin = loader.create("SenderPlugin");
        }
        catch(Poco::Exception& e){
            log_error("Library load error, plugin from "+libName+" not work ");
            log_error(e.name());

        }
        catch(std::exception& e){
            log_error("Library load error, plugin from "+libName+" not work ");
            log_error(e.what());
        }
    }
    ~LibraryLoader(){
        if(pPlugin!=nullptr){
            loader.classFor("SenderPlugin").autoDelete(pPlugin);
            loader.unloadLibrary(libName);
        }
    }
    ISender* getInstanse(){
        return pPlugin;
    }
};
