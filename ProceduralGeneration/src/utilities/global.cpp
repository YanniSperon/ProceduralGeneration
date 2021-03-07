#include "Global.h"

static std::chrono::steady_clock::time_point g_SystemStartTime;

void Global::Initialize()
{
	g_SystemStartTime = std::chrono::high_resolution_clock::now();
}

std::chrono::steady_clock::time_point& Global::GetStartTime()
{
	return g_SystemStartTime;
}
