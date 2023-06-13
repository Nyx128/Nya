#pragma once
#include "pch.hpp"
#include "logging/NYLogger.hpp"

namespace Nya {
	class NYTimer {
	public:
		NYTimer();
		~NYTimer();

		void endTimer();

		inline float getSeconds() {
			if (complete) { return tseconds; }
			else { NYLogger::logWarning("endTimer() hasn't been called"); }
		}
		inline float getMillis() {
			if (complete) { return tmillis; }
			else { NYLogger::logWarning("endTimer() hasn't been called"); }
		}
		inline float getMicros() {
			if (complete) { return tmicros; }
			else { NYLogger::logWarning("endTimer() hasn't been called"); }
		}

	private:
		std::chrono::high_resolution_clock::time_point start;
		std::chrono::high_resolution_clock::time_point end;
		std::chrono::duration<float> duration;
		float tseconds = 0.0f;
		float tmillis = 0.0f;
		float tmicros = 0.0f;

		bool complete = false;
	};
}

