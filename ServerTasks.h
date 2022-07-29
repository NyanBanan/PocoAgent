#pragma once

#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/DateTimeFormatter.h"

#include "SenderPlugin/ISender.h"
#include "Poco/ClassLoader.h"
#include "Poco/Manifest.h"
#include <iostream>
#define SCAN_INTERVAL 5000
typedef Poco::ClassLoader<ISender> PluginLoader;
typedef Poco::Manifest<ISender> PluginManifest;

class myServerTask: public Poco::Task
{
	PluginLoader loader;
	ISender* pPlugin;
	std::string libName;
	std::string available_interfaces;
	//Poco::SharedPtr<PocoAgent> agent;
public:
	

	myServerTask(); //Регистрируем задачу под именем "MyTask"
	~myServerTask();
	//Запуск задачи
	void runTask();
	void clear();
	std::string getAvailableInterfaces();
};
