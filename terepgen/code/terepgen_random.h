#if !defined(TEREPGEN_RANDOM_H)

#define RANDOM_TEX_SIZE 33

struct value_noise_generator
{
    uint32 Seed;
    dynamic_grid3D RandomTex;
};

struct perlin_noise_generator
{
    uint32 Seed;
    v2 GradientTexV2[RANDOM_TEX_SIZE*RANDOM_TEX_SIZE];
    v3 GradientTex[RANDOM_TEX_SIZE*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE];
};

struct perlin_noise_array
{
    perlin_noise_generator Noise[3];
};

#define TEREPGEN_RANDOM_H
#endif