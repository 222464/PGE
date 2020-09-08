#include "NoiseGenerator.h"

#include "Math.h"
#include <algorithm>
#include <cmath>

#include <assert.h>

using namespace pge;

NoiseGenerator::NoiseGenerator()
: seed(1), smoothDist(1)
{}

// Constant seed pseudo-random generator based on the one from http://freespace.virgin.net/hugo.elias/models/_perlin.htm
// Cannot use normal random number generator, since we want it to operate on a single parameter to produce
// the same values whenever the parameters are the same. Seeds are done by adding on offset to that value
float NoiseGenerator::noise(int value) {
	// Add the seed offset
	value += varyingInternalSeed;

	// Random bit shifts to produce a pseudo-random number
	value = (value << 13) ^ value;

	// Random stuff to "mess up" the number
	return 1.0f - ((value * (value * value * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
}

// The previous noise is one dimensional, but can be used for 2D if simply get a condensed,
// 1 dimensional, rarely repeating (for different x and y coords) value from the coordinates in a non-random way
// e.g. add them at different rates (*57 is for a different rate): value = x + y * 57
float NoiseGenerator::noise2D(int x, int y) {
	return noise(x + y * 577);
}

// The previous noise is one dimensional, but can be used for 3D if simply get a condensed,
// 1 dimensional, rarely repeating (for different x and y coords) value from the coordinates in a non-random way
// e.g. add them at different rates (*57 is for a different rate): value = x + y * 57
inline float NoiseGenerator::noise3D(int x, int y, int z) {
	return noise(x + y * 577 + z * 21529);
}

// Interpolation function for interpolating the results from the noise function to make it coherent
// Using the cosine interpolation method to curve the coefficient of interpolation before using it to perform a Lerp
float NoiseGenerator::interpolateCosine(float val1, float val2, float coeff_interpolate) {
	// Range check
	//assert(coeff_interpolate >= 0.0f && coeff_interpolate <= 1.0f);

	// Use coefficient of interpolation to get angle in range [0, PI]
	float coeff_angle = coeff_interpolate * pi;

	// Cosine of angle converted to [0, 1] range to serve as the new coefficient of interpolation
	float clampedCos = (1.0f - cosf(coeff_angle)) * 0.5f;

	// Use this as the new "curved" coefficient of interpolation for a linear interpolation between the two values
	return val1 * (1.0f - clampedCos) + val2 * clampedCos;
}

// Smooth out the noise by blending the noise value with other noise values right around it
// Flattens the noise a little though. Oh well
float NoiseGenerator::smoothNoise3D(int x, int y, int z) {
	float sum = 0.0f;

	// Get surrounding noise values, divide their values based on their distance from the center, and add them on
	for (int deltaX = -smoothDist; deltaX <= smoothDist; deltaX++)
	for (int deltaY = -smoothDist; deltaY <= smoothDist; deltaY++)
	for (int deltaZ = -smoothDist; deltaZ <= smoothDist; deltaZ++) {
		if (deltaX == 0 && deltaY == 0)
			sum += noise3D(x, y, z) * 0.25f;
		else {
			float dist = sqrtf(static_cast<float>(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ)) * 2.0f;
			sum += noise3D(x + deltaX, y + deltaY, z + deltaZ) / (dist * dist);
		}
	}

	return sum;
}

float NoiseGenerator::interpolatedNoise3D(float x, float y, float z) {
	// Noise goes in integral increments, so blend in between those with the interpolating functions
	// Interpolation coefficients are therefore the decimal portions of the floats
	int xi, yi, zi;
	float coeff_interpolation_x, coeff_interpolation_y, coeff_interpolation_z;

	if (x < 0.0f) {
		xi = static_cast<int>(x - 1.0f);
		coeff_interpolation_x = x - static_cast<float>(xi);
	}
	else {
		xi = static_cast<int>(x);
		coeff_interpolation_x = x - static_cast<float>(xi);
	}

	if (y < 0.0f) {
		yi = static_cast<int>(y - 1.0f);
		coeff_interpolation_y = y - static_cast<float>(yi);
	}
	else {
		yi = static_cast<int>(y);
		coeff_interpolation_y = y - static_cast<float>(yi);
	}

	if (z < 0.0f) {
		zi = static_cast<int>(z - 1.0f);
		coeff_interpolation_z = z - static_cast<float>(zi);
	}
	else {
		zi = static_cast<int>(z);
		coeff_interpolation_z = z - static_cast<float>(zi);
	}

	// Integral noise values
	float x_lower_z_lower = noise3D(xi, yi, zi);
	float x_higher_z_lower = noise3D(xi + 1, yi, zi);
	float y_lower_z_lower = noise3D(xi, yi + 1, zi);
	float y_higher_z_lower = noise3D(xi + 1, yi + 1, zi);
	float x_lower_z_higher = noise3D(xi, yi, zi + 1);
	float x_higher_z_higher = noise3D(xi + 1, yi, zi + 1);
	float y_lower_z_higher = noise3D(xi, yi + 1, zi + 1);
	float y_higher_z_higher = noise3D(xi + 1, yi + 1, zi + 1);

	// First interpolate the different x integral values by the x coefficient
	float x_interp_y_lower_z_lower = interpolateCosine(x_lower_z_lower, x_higher_z_lower, coeff_interpolation_x);
	float x_interp_y_higher_z_lower = interpolateCosine(y_lower_z_lower, y_higher_z_lower, coeff_interpolation_x);
	float x_interp_y_lower_z_higher = interpolateCosine(x_lower_z_higher, x_higher_z_higher, coeff_interpolation_x);
	float x_interp_y_higher_z_higher = interpolateCosine(y_lower_z_higher, y_higher_z_higher, coeff_interpolation_x);

	float y_interp_z_lower = interpolateCosine(x_interp_y_lower_z_lower, x_interp_y_higher_z_lower, coeff_interpolation_y);
	float y_interp_z_higher = interpolateCosine(x_interp_y_lower_z_higher, x_interp_y_higher_z_higher, coeff_interpolation_y);

	// Then interpolate the different y integral values by the y coefficient and return
	return interpolateCosine(y_interp_z_lower, y_interp_z_higher, coeff_interpolation_z);
}

float NoiseGenerator::interpolatedSmoothNoise3D(float x, float y, float z) {
	// Noise goes in integral increments, so blend in between those with the interpolating functions
	// Interpolation coefficients are therefore the decimal portions of the floats
	int xi, yi, zi;
	float coeff_interpolation_x, coeff_interpolation_y, coeff_interpolation_z;

	if (x < 0.0f) {
		xi = static_cast<int>(x - 1.0f);
		coeff_interpolation_x = x - static_cast<float>(xi);
	}
	else {
		xi = static_cast<int>(x);
		coeff_interpolation_x = x - static_cast<float>(xi);
	}

	if (y < 0.0f) {
		yi = static_cast<int>(y - 1.0f);
		coeff_interpolation_y = y - static_cast<float>(yi);
	}
	else {
		yi = static_cast<int>(y);
		coeff_interpolation_y = y - static_cast<float>(yi);
	}

	if (z < 0.0f) {
		zi = static_cast<int>(z - 1.0f);
		coeff_interpolation_z = z - static_cast<float>(zi);
	}
	else {
		zi = static_cast<int>(z);
		coeff_interpolation_z = z - static_cast<float>(zi);
	}

	// Integral noise values
	float x_lower_z_lower = smoothNoise3D(xi, yi, zi);
	float x_higher_z_lower = smoothNoise3D(xi + 1, yi, zi);
	float y_lower_z_lower = smoothNoise3D(xi, yi + 1, zi);
	float y_higher_z_lower = smoothNoise3D(xi + 1, yi + 1, zi);
	float x_lower_z_higher = smoothNoise3D(xi, yi, zi + 1);
	float x_higher_z_higher = smoothNoise3D(xi + 1, yi, zi + 1);
	float y_lower_z_higher = smoothNoise3D(xi, yi + 1, zi + 1);
	float y_higher_z_higher = smoothNoise3D(xi + 1, yi + 1, zi + 1);

	// First interpolate the different x integral values by the x coefficient
	float x_interp_y_lower_z_lower = interpolateCosine(x_lower_z_lower, x_higher_z_lower, coeff_interpolation_x);
	float x_interp_y_higher_z_lower = interpolateCosine(y_lower_z_lower, y_higher_z_lower, coeff_interpolation_x);
	float x_interp_y_lower_z_higher = interpolateCosine(x_lower_z_higher, x_higher_z_higher, coeff_interpolation_x);
	float x_interp_y_higher_z_higher = interpolateCosine(y_lower_z_higher, y_higher_z_higher, coeff_interpolation_x);

	float y_interp_z_lower = interpolateCosine(x_interp_y_lower_z_lower, x_interp_y_higher_z_lower, coeff_interpolation_y);
	float y_interp_z_higher = interpolateCosine(x_interp_y_lower_z_higher, x_interp_y_higher_z_higher, coeff_interpolation_y);

	// Then interpolate the different y integral values by the y coefficient and return
	return interpolateCosine(y_interp_z_lower, y_interp_z_higher, coeff_interpolation_z);
}

// Added noises (number of noises is known as the number of octaves) together with different
// persistence values (how much the effect the result in terms of amplitude and frequency) to create the final perlin noise
float NoiseGenerator::perlinNoise3D(float x, float y, float z, int octaves, float frequencyBase, float persistence) {
	float result = 0;

	// Add octaves
	for (int oct = 0; oct < octaves; oct++) {
		// Different internal seed for every octave so it does not repeated
		varyingInternalSeed = seed + oct * 3792;

		// The higher the octave, the higher the frequency, and the lower the amplitude
		// The frequency multiplier is multiplied by the coordinates to increase/decrease the wave frequency
		// The result of the noise is multiplied by the amplitude to determine how much it affects the final result
		float frequency = 1.0f / powf(frequencyBase, static_cast<float>(oct));
		float amplitude = powf(persistence, static_cast<float>(oct));

		result += interpolatedNoise3D(x * frequency, y * frequency, z * frequency) * amplitude;
	}

	return result;
}

float NoiseGenerator::smoothedPerlinNoise3D(float x, float y, float z, int octaves, float frequencyBase, float persistence) {
	float result = 0;

	// Add octaves
	for (int oct = 0; oct < octaves; oct++) {
		// Different internal seed for every octave so it does not repeat
		varyingInternalSeed = seed + oct * 3792;

		// The higher the octave, the higher the frequency, and the lower the amplitude
		// The frequency multiplier is multiplied by the coordinates to increase/decrease the wave frequency
		// The result of the noise is multiplied by the amplitude to determine how much it affects the final result
		float frequency = 1.0f / powf(frequencyBase, static_cast<float>(oct));
		float amplitude = powf(persistence, static_cast<float>(oct));

		result += interpolatedSmoothNoise3D(x * frequency, y * frequency, z * frequency) * amplitude;
	}

	return result;
}