#include "../stdafx.h"
#include "timer.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
Timer::Timer()
{
	startPoint_ = std::chrono::high_resolution_clock::now();
}
//======================================================================================================================
void Timer::Stop()
{
	auto endPoint = std::chrono::high_resolution_clock::now();
	auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startPoint_).time_since_epoch().count();
	auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();
	auto duration = end - start;

	double ms = duration * 0.001;
	std::cout << "duration: " << duration << "ms" << "   " << static_cast<double>(duration) / 1'000'000 << "s\n";
}
//======================================================================================================================
Timer::~Timer()
{
	Stop();
}

}