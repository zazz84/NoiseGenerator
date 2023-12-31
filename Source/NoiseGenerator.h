/*
  ==============================================================================

    NoiseGenerator.h
    Created: 30 Dec 2023 9:06:31pm
    Author:  zazz

  ==============================================================================
*/

#pragma once

#include <stdlib.h>
#include <math.h>

//==============================================================================
class RandomNoiseGenerator
{
public:
	RandomNoiseGenerator() {};

	float process()
	{
		return (2.0f * rand() / RAND_MAX) - 1.0f;
	};
};

//==============================================================================
class FastNoiseGenerator
{
public:
	FastNoiseGenerator() {};

	float process()
	{
		m_x1 ^= m_x2;
		const float out = m_x2 / 2147483647.0f;
		m_x2 += m_x1;
		return out;
	};

private:
	int m_x1 = 0x67452301;
	int m_x2 = 0xefcdab89;
};

//==============================================================================
class InterpolationNoiseGenerator
{
public:
	InterpolationNoiseGenerator() {};

	void init(int sampleRate)
	{
		m_sampleRate = sampleRate;
		setSpeed(0.5f);
		setTarget();
	};
	void setTarget()
	{
		m_targetValue = m_random.process();
		const float speed = m_interpolationSpeed + m_random.process() * m_interpolationSpeedRandom;
		m_speed = speed * (m_targetValue - m_currentValue) / m_sampleRate;
	};
	void setSpeed(float speedCoef)
	{
		m_interpolationSpeed = 100.0f + speedCoef * m_sampleRate * 0.49f;
		m_interpolationSpeedRandom = m_interpolationSpeed * 0.8f;
	};
	float process()
	{
		m_currentValue += m_speed;
		
		if (fabsf(m_currentValue) > fabsf(m_targetValue))
		{
			m_currentValue = m_targetValue;
			setTarget();
			return m_currentValue;
		}
	};

private:
	int m_sampleRate;

	FastNoiseGenerator m_random = {};
	float m_currentValue = 0.0f;
	float m_targetValue = 0.0f;
	float m_speed = 0.0f;

	float m_interpolationSpeed = 0.0f;
	float m_interpolationSpeedRandom = 0.0f;
};