#pragma once

namespace pge {
	const float pi = 3.14159265359f;
	const float piOver2 = 0.5f * pi;
	const float piOver4 = 0.25f * pi;
	const float piTimes2 = 2.0f * pi;

	template<class T>
	T clamp(T value, T min, T max) {
		if (value < min)
			return min;

		if (value > max)
			return max;

		return value;
	}

	template<class T>
	T sign(T value) {
		return value < 0.0f ? -1.0f : 1.0f;
	}

	template<class T>
	T lerp(T s, T e, T l) {
		return (e - s) * l + s;
	}
}