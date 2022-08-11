#ifndef LOGGER
#define LOGGER
#include <iostream>
#include <functional>
#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/File.h"

#define log_fatal(msg) \
	if ((Poco::Logger::root()).fatal()) (Poco::Logger::root()).fatal(msg, __FILE__, __LINE__); else (void) 0

#define log_critical(msg) \
	if ((Poco::Logger::root()).critical()) (Poco::Logger::root()).critical(msg, __FILE__, __LINE__); else (void) 0

#define log_error(msg) \
	if ((Poco::Logger::root()).error()) (Poco::Logger::root()).error(msg, __FILE__, __LINE__); else (void) 0

#define log_warning(msg) \
	if ((Poco::Logger::root()).warning()) (Poco::Logger::root()).warning(msg, __FILE__, __LINE__); else (void) 0

#define log_notice(msg) \
	if ((Poco::Logger::root()).notice()) (Poco::Logger::root()).notice(msg, __FILE__, __LINE__); else (void) 0

#define log_information(msg) \
	if ((Poco::Logger::root()).information()) (Poco::Logger::root()).information(msg, __FILE__, __LINE__); else (void) 0

void init_logger();

#endif