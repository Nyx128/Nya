#include "pch.hpp"
#include "NYLogger.hpp"
#include <cassert>

//TODO: variadic args
namespace Nya {
	void NYLogger::logMessage(const char* message, NYLogLevel logLevel, va_list arg_list){
		//get the standard output handle using windows.h
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		//change the color used to print
		//index into the afformentioned msgColors vector with the logLevel enum
		SetConsoleTextAttribute(h, msgColors[logLevel]);
		//print the tag in front of the message
		printf("%s", Tags[logLevel]);
		//print the formatted message
		vfprintf(stdout, message, arg_list);
		printf("\n");

		SetConsoleTextAttribute(h, defaultCol);
	}

	//pretty straightforward
	void NYLogger::logFatal(const char* message, ...){
		//use the message as format to make an arg_list to pass to the logMessage function
		va_list arg_list;
		va_start(arg_list, message);
		logMessage(message, NY_LOG_LEVEL_FATAL, arg_list);
		va_end(arg_list);
		assert(false);
	}

	void NYLogger::logError(const char* message, ...){
		va_list arg_list;
		va_start(arg_list, message);
		logMessage(message, NY_LOG_LEVEL_ERROR, arg_list);
		va_end(arg_list);
		assert(false);
	}

	void NYLogger::logWarning(const char* message, ...){
		va_list arg_list;
		va_start(arg_list, message);
		logMessage(message, NY_LOG_LEVEL_WARNING, arg_list);
		va_end(arg_list);

	}

	void NYLogger::logInfo(const char* message, ...){
		va_list arg_list;
		va_start(arg_list, message);
		logMessage(message, NY_LOG_LEVEL_INFO, arg_list);
		va_end(arg_list);

	}

	void NYLogger::logTrace(const char* message, ...){
		va_list arg_list;
		va_start(arg_list, message);
		logMessage(message, NY_LOG_LEVEL_TRACE, arg_list);
		va_end(arg_list);

	}

	void NYLogger::checkAssert(bool condition, const char* message) {
#ifdef NY_DEBUG
		if (!condition) {
			std::string traceInfo = " [line: %d, file: %s]";
			std::string logMsg = message + traceInfo;
			logFatal(logMsg.c_str(), __LINE__, __FILE__);
			assert(false);
		}
#endif
		
	}
}