#pragma once
#include "Poco/ClassLoader.h"
#include "Poco/Manifest.h"
#include "../Plugins/AbstractPlugin.h"
#include <iostream>
#include "../Logger.h"

typedef Poco::ClassLoader<AbstractPlugin> PluginLoader;
typedef Poco::Manifest<AbstractPlugin> PluginManifest;

class LibraryLoader
{
private:
    PluginLoader loader;
    AbstractPlugin* pPlugin=nullptr;
	std::string libName;
public:
    LibraryLoader(){
        libName = "Plugins/libPlugin";
        libName += Poco::SharedLibrary::suffix(); // append .dll or .so
        try {
            loader.loadLibrary(libName);
            pPlugin = loader.create("PluginGetInterfaces");
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
            loader.classFor("PluginGetInterfaces").autoDelete(pPlugin);
            loader.unloadLibrary(libName);
        }
    }
    AbstractPlugin* getInstanse(){
        return pPlugin;
    }
};
