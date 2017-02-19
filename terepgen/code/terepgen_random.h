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

void SetSeed(perlin_noise_generator *Generator, uint32 NewSeed);
real32 RandomFloat(perlin_noise_generator *Generator, real32 Row, real32 Column);
real32 RandomFloat(perlin_noise_generator *Generator, real32 Plane, real32 Row, real32 Column);
real32 RandomFloat(perlin_noise_generator *Generator, v3 WorldPos);

#define TEREPGEN_RANDOM_H
#endif