#pragma once

#include <chrono>

namespace xengine
{

class Timer final
{
public:
	Timer();
	virtual ~Timer();

private:
	void	Stop();

	std::chrono::time_point<std::chrono::high_resolution_clock> startPoint_;
};

}