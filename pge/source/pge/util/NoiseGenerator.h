#pragma once

namespace pge {
    class NoiseGenerator {
    private:
        int varyingInternalSeed;

    public:
        int seed;
        int smoothDist;

        NoiseGenerator();

        float noise(int value);
        float noise2D(int x, int y);
        float noise3D(int x, int y, int z);
        float interpolateCosine(float val1, float val2, float coeff_interpolate);
        float smoothNoise3D(int x, int y, int z);
        float interpolatedNoise3D(float x, float y, float z);
        float interpolatedSmoothNoise3D(float x, float y, float z);
        float perlinNoise3D(float x, float y, float z, int octaves, float frequencyBase, float persistence);
        float smoothedPerlinNoise3D(float x, float y, float z, int octaves, float frequencyBase, float persistence);
    };
}