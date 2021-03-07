#pragma once

#include <chrono>

class Global {
public:
	static void Initialize();
	static std::chrono::steady_clock::time_point& GetStartTime();
};
