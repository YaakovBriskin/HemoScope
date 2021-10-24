#pragma once

#include <chrono>

// Time is measured in seconds
class Timer
{
public:
	Timer() = default;
	~Timer() = default;

	void start()
	{
		m_timeStart = std::chrono::high_resolution_clock::now();
	}

	void end()
	{
		m_timeEnd = std::chrono::high_resolution_clock::now();
		m_duration = std::chrono::duration_cast<std::chrono::duration<double>>(m_timeEnd - m_timeStart).count();
	}

	double getDuration()
	{
		return m_duration;
	}

	size_t getDurationSeconds()
	{
		return (size_t)floor(m_duration + 0.5);
	}

	size_t getDurationMilliseconds()
	{
		return (size_t)floor(1000.0 * m_duration + 0.5);
	}

private:
	std::chrono::high_resolution_clock::time_point m_timeStart{};
	std::chrono::high_resolution_clock::time_point m_timeEnd{};
	double m_duration{};
};
