#pragma once
#define SCAN_INTERVAL 10000
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/DateTimeFormatter.h"

#include "SenderPlugin/ISender.h"
#include "Poco/ClassLoader.h"
#include "Poco/Manifest.h"
#include <iostream>

typedef Poco::ClassLoader<ISender> PluginLoader;
typedef Poco::Manifest<ISender> PluginManifest;

class myServerTask: public Poco::Task
{
	PluginLoader loader;
	Poco::SharedPtr<ISender> pPlugin;
	std::string libName;
	std::string available_interfaces="None";
	//Poco::SharedPtr<PocoAgent> agent;
public:
	myServerTask(); //Регистрируем задачу под именем "MyTask"
	~myServerTask();
	//Запуск задачи
	void runTask();
    void cancel();
	std::string getAvailableInterfaces();
};