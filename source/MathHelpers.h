#pragma once
#include <cmath>

namespace dae
{
	/* --- HELPER STRUCTS --- */
	struct Int2
	{
		int x{};
		int y{};
	};

	/* --- CONSTANTS --- */
	constexpr auto PI = 3.14159265358979323846f;
	constexpr auto PI_DIV_2 = 1.57079632679489661923f;
	constexpr auto PI_DIV_4 = 0.785398163397448309616f;
	constexpr auto PI_2 = 6.283185307179586476925f;
	constexpr auto PI_4 = 12.56637061435917295385f;

	constexpr auto TO_DEGREES = (180.0f / PI);
	constexpr auto TO_RADIANS(PI / 180.0f);

	/* --- HELPER FUNCTIONS --- */
	inline float Square(float a)
	{
		return a * a;
	}

	inline float Lerpf(float a, float b, float factor)
	{
		return ((1 - factor) * a) + (factor * b);
	}

	inline bool AreEqual(float a, float b, float epsilon = FLT_EPSILON)
	{
		return abs(a - b) < epsilon;
	}

	inline int Clamp(const int v, int min, int max)
	{
		if(v < min) return min;
		if(v > max) return max;
		return v;
	}

	inline float Clamp(const float v, float min, float max)
	{
		if(v < min) return min;
		if(v > max) return max;
		return v;
	}

	inline float Wrap(const float input, float lowerBound, float upperBound)
	{
		// Wraps the value back around to the lowerbound when overflowing, also in reverse back to upperbound
		// Example: Wrap(5, 0, 10) = 5
		// Example: Wrap(15, 0, 10) = 5
		// Example: Wrap(-5, 0, 10) = 5

		float range = upperBound - lowerBound;
		if(input < lowerBound)
			return upperBound - fmod(lowerBound - input, range);  // Fmod returns remainder of division
		return lowerBound + fmod(input - lowerBound, range);
	}

	inline float Saturate(const float v)
	{
		if(v < 0.f) return 0.f;
		if(v > 1.f) return 1.f;
		return v;
	}
}