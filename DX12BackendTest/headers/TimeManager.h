#pragma once
#include <chrono>

class TimeManager
{
public:
	TimeManager() : 
		m_Deltatime(0.016f)
	{
		m_StartTimePoint = m_Clock.now();
		m_RecordedTimePoint = m_Clock.now();
	}
	float deltaTime() const
	{
		return m_Deltatime.count();
	}
	void UpdateClock()
	{
		std::chrono::high_resolution_clock::time_point new_point = m_Clock.now();
		//m_Deltatime = std::chrono::duration_cast<std::chrono::duration<float>>(new_point - m_RecordedTimePoint);
		m_Deltatime = (new_point - m_RecordedTimePoint);
		m_RecordedTimePoint = new_point;
	}
	float elapsedTime() const
	{
		return std::chrono::duration_cast<std::chrono::duration<float>>(m_RecordedTimePoint - m_StartTimePoint).count();
	}
private:
	std::chrono::high_resolution_clock::time_point m_StartTimePoint;
	std::chrono::duration<float> m_Deltatime;
	std::chrono::high_resolution_clock m_Clock;
	std::chrono::high_resolution_clock::time_point m_RecordedTimePoint;
};