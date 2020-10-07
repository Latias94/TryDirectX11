#include "ChiliTimer.h"

using namespace std::chrono;

ChiliTimer::ChiliTimer() noexcept
{
	last = steady_clock::now();
}

// 从上次调用 Mark 起，到现在所经历的时间
float ChiliTimer::Mark() noexcept
{
	const auto old = last;
	last = steady_clock::now();
	const duration<float> frameTime = last - old;
	return frameTime.count();
}

// 从上次调用 Mark 起，到现在所经历的时间，但不会改 last 变量
float ChiliTimer::Peek() const noexcept
{
	return duration<float>(steady_clock::now() - last).count();
}