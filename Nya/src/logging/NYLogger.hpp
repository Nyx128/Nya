#pragma once
#include "pch.hpp"

namespace Nya{
	//this class only contains static methods for logging
	class NYLogger {
	public:
		//levels of log
		enum NYLogLevel {
			NY_LOG_LEVEL_FATAL = 0,
			NY_LOG_LEVEL_ERROR = 1,
			NY_LOG_LEVEL_WARNING = 2,
			NY_LOG_LEVEL_INFO = 3,
			NY_LOG_LEVEL_TRACE = 4
		};

		NYLogger() {};
		~NYLogger() {};

		//methods to be used to log stuff

		//logs fatal messages and stops execution
		static void logFatal(const char* message, ...);
		//logs errors and stops execution
		static void logError(const char* message, ...);
		static void logWarning(const char* message, ...);
		static void logInfo(const char* message, ...);
		static void logTrace(const char* message, ...);

		//standard assert with a message, stops execution on failure
		static void checkAssert(bool condition, const char* message);
	private:
		//private method used in all the log methods
		static void logMessage(const char* message, NYLogLevel logLevel, va_list arg_list);

		//default console color , white(unintensified)
		static const uint32_t defaultCol = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;

		//the colors correspond to the log levels with the int value of the enum as index
		static const inline std::vector<uint32_t> msgColors = {
			//red intensified background, and intensified white foreground for fatal logs 
			BACKGROUND_RED | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
			//red intensified foreground for errors
			FOREGROUND_RED | FOREGROUND_INTENSITY,
			//intensified yelloe foreground for warnings
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
			//intensified blue for info logs
			FOREGROUND_BLUE| FOREGROUND_INTENSITY,
			//intensified green for trace 
			FOREGROUND_GREEN| FOREGROUND_INTENSITY
		};

		//tags to be used before each message is logged, it again corresponds to the log levels in the enum
		static const inline std::vector<const char*> Tags = {
			"[FATAL]:",
			"[ERROR]:",
			"[WARNING]:",
			"[INFO]:",
			"[TRACE]:"
		};
	};
}


