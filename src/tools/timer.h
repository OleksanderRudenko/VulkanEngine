#pragma once

#include <chrono>

//**********************************************************************************************************************
//	Timer
//----------------------------------------------------------------------------------------------------------------------
class Timer final
{
public:
	Timer();
	virtual ~Timer();

private:
	void	Stop();

	std::chrono::time_point<std::chrono::high_resolution_clock> startPoint_;
};