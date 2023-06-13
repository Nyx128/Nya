#include "pch.hpp"
#include "NYTimer.hpp"

namespace Nya {
	NYTimer::NYTimer() {
		start = std::chrono::high_resolution_clock::now();
	}

	void NYTimer::endTimer() {
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		tseconds = duration.count();
		tmillis = tseconds * 1000.0f;
		tmicros = tseconds * 1000000.0f;

		complete = true;
	}

	NYTimer::~NYTimer() {

	}
}

