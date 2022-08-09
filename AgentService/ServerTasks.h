#pragma once
#include "RESTInter.h"

#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/DateTimeFormatter.h"
#include <Poco/Util/Application.h>
#include "LibraryLoader.h"
#define SCAN_INTERVAL 5000


class myServerTask: public Poco::Task
{
public:
	RESTinter* rest;
	explicit myServerTask(RESTinter* _rest);
	~myServerTask();
	//Запуск задачи
	void runTask() override;
};
