#pragma once

namespace pge {
	const float _pi = 3.14159265359f;
	const float _piOver2 = 0.5f * _pi;
	const float _piOver4 = 0.25f * _pi;
	const float _piTimes2 = 2.0f * _pi;

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