#if !defined(TEREPGEN_RANDOM_H)

#define RANDOM_TEX_SIZE 33

struct perlin_noise_generator
{
    uint32 Seed;
    // NOTE: for 2d randoming
    v2 GradientTexV2[RANDOM_TEX_SIZE*RANDOM_TEX_SIZE];
    // NOTE: for 3d randoming
    v3 GradientTex[RANDOM_TEX_SIZE*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE];
};

struct perlin_noise_array
{
    perlin_noise_generator Noise[3];
};

#define TEREPGEN_RANDOM_H
#endif