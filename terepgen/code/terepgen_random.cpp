/*
    Terep generátor by Hidvégi Máté @2015

*/

// NOTE: Mersenne twister from http://www.eternallyconfuzzled.com/tuts/algorithms/jsw_tut_rand.aspx
#define MT_N 624
#define MT_M 397
#define MT_A 0x9908b0dfUL
#define MT_U 0x80000000UL
#define MT_L 0x7fffffffUL

static unsigned long MTx[MT_N];
static int MTNext;

internal void 
MtSeed(unsigned long s)
{
    int i;
    
    MTx[0] = s & 0xffffffffUL;
    
    for (i = 1; i < MT_N; i++) {
        MTx[i] = (1812433253UL * (MTx[i - 1] ^ (MTx[i - 1] >> 30)) + i);
        MTx[i] &= 0xffffffffUL;
    }
}

// NOTE: Gives back a random long integer in [0-max_long]
internal unsigned long 
MtRand(void)
{
    unsigned long y, a;
    int i;

    /* Refill MTx if exhausted */
    if (MTNext == MT_N) {
        MTNext = 0;

        for (i = 0; i < MT_N - 1; i++) {
            y = (MTx[i] & MT_U) | MTx[i + 1] & MT_L;
            a = (y & 0x1UL) ? MT_A : 0x0UL;
            MTx[i] = MTx[(i + MT_M) % MT_N] ^ (y >> 1) ^ a;
        }

        y = (MTx[MT_N - 1] & MT_U) | MTx[0] & MT_L;
        a = (y & 0x1UL) ? MT_A : 0x0UL;
        MTx[MT_N - 1] = MTx[MT_M - 1] ^ (y >> 1) ^ a;
    }

    y = MTx[MTNext++];
    
    /* Improve distribution */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    
    return y;
}

// NOTE: Gives back a random real number in [(-1.0) - (1.0)]
internal real32 
MtStandardDeviate(void)
{
    real32 Result = ((real32)MtRand() / (real32)0xffffffffUL) * 2.0f - 1.0f;
    Assert(Result < 1.0f && Result > -1.0f);
    return Result;
}

// Value noise

inline real32
Lerp(real32 A, real32 B, real32 X)
{
    real32 T = X*X*X*(10.0f + (X*((X*6.0f) - 15.0f) ) );
    real32 Result = A + (T*(B - A));
    return Result;
}

        
void SetSeed(value_noise_generator *Generator, uint32 NewSeed)
{
    Generator->Seed = NewSeed;
    uint32 TexSize = Generator->RandomTex.Dimension * Generator->RandomTex.Dimension * Generator->RandomTex.Dimension;
    MtSeed(NewSeed);
    for(uint32 Index = 0; Index < TexSize; Index++)
    {
        real32 Rand = MtStandardDeviate();
        Generator->RandomTex.Elements[Index] = Rand;
    }
}
        
internal real32 
RandomFloat(value_noise_generator *Generator, real32 Plane, real32 Row, real32 Column)
{
    real32 PlaneMod = ModReal32(Plane, (real32)RANDOM_TEX_SIZE);
    uint32 PlaneWhole = FloorInt32(PlaneMod);
    uint32 PlaneWhole2 = (PlaneWhole+1) % RANDOM_TEX_SIZE;
    real32 PlaneRemainder = PlaneMod - (real32)PlaneWhole;
    
    real32 RowMod = ModReal32(Row, (real32)RANDOM_TEX_SIZE);
    uint32 RowWhole = FloorInt32(RowMod);
    uint32 RowWhole2 = (RowWhole+1) % RANDOM_TEX_SIZE;
    real32 RowRemainder = RowMod - (real32)RowWhole;
    
    real32 ColumnMod = ModReal32(Column, (real32)RANDOM_TEX_SIZE);
    uint32 ColumnWhole = FloorInt32(ColumnMod);
    uint32 ColumnWhole2 = (ColumnWhole+1) % RANDOM_TEX_SIZE;
    real32 ColumnRemainder = ColumnMod - (real32)ColumnWhole;
    
    real32 R000 = Generator->RandomTex.GetPRC(PlaneWhole , RowWhole , ColumnWhole );
    real32 R001 = Generator->RandomTex.GetPRC(PlaneWhole , RowWhole , ColumnWhole2);
    real32 R010 = Generator->RandomTex.GetPRC(PlaneWhole , RowWhole2, ColumnWhole );
    real32 R011 = Generator->RandomTex.GetPRC(PlaneWhole , RowWhole2, ColumnWhole2);
    real32 R100 = Generator->RandomTex.GetPRC(PlaneWhole2, RowWhole , ColumnWhole );
    real32 R101 = Generator->RandomTex.GetPRC(PlaneWhole2, RowWhole , ColumnWhole2);
    real32 R110 = Generator->RandomTex.GetPRC(PlaneWhole2, RowWhole2, ColumnWhole );
    real32 R111 = Generator->RandomTex.GetPRC(PlaneWhole2, RowWhole2, ColumnWhole2);
    
    real32 I00 = Lerp(R000, R001, ColumnRemainder);
    real32 I01 = Lerp(R010, R011, ColumnRemainder);
    real32 I10 = Lerp(R100, R101, ColumnRemainder);
    real32 I11 = Lerp(R110, R111, ColumnRemainder);
    
    real32 I0 = Lerp(I00, I01, RowRemainder);
    real32 I1 = Lerp(I10, I11, RowRemainder);
    
    real32 Result = Lerp(I0, I1, PlaneRemainder);
    return Result;
}

internal real32 
RandomFloat(value_noise_generator *Generator, v3 WorldPos)
{
    real32 Result = RandomFloat(Generator, WorldPos.X, WorldPos.Y, WorldPos.Z);
    return Result;
}

// Perlin noise

internal void 
SetSeed(perlin_noise_generator *Generator, uint32 NewSeed)
{
    Generator->Seed = NewSeed;
    uint32 TexSize = ArrayCount(Generator->GradientTex);
    MtSeed(NewSeed);
    v3 PseudoGradientTable[12] 
    { 
        { -1, -1, 0},{ -1, 1, 0},{ 1, -1, 0},{ 1, 1, 0},
        { -1, 0, -1},{ -1, 0, 1},{ 1, 0, -1},{ 1, 0, 1},
        { 0, -1, -1},{ 0, -1, 1},{ 0, 1, -1},{ 0, 1, 1}
    };
    for (uint32 Index = 0; Index < TexSize; Index++)
    {
        uint32 Rand = MtRand();
        Generator->GradientTex[Index] = PseudoGradientTable[Rand % 12];
    }
    
    TexSize = ArrayCount(Generator->GradientTexV2);
    for(uint32 Index = 0; Index < TexSize; Index++)
    {
        real32 X = MtStandardDeviate();
        real32 Y = Sqrt(1.0f - X*X);
        uint32 YSign = (uint32)(MtRand() & 1);
        if(YSign)
        {
            Y = -Y;
        }
        v2 Gradient = { X, Y };
        real32 Len = Length(Gradient);
        Assert(Len >= 0.9999f && Len <= 1.0001);
        Generator->GradientTexV2[Index] = v2{X, Y}; 
    }
}

internal real32 
RandomFloat(perlin_noise_generator *Generator, real32 Row, real32 Column)
{
    real32 RowMod = ModReal32(Row, (real32)RANDOM_TEX_SIZE);
    uint32 RowWhole = FloorInt32(RowMod);
    uint32 RowWhole2 = (RowWhole + 1) % RANDOM_TEX_SIZE;
    real32 RowRemainder = RowMod - (real32)RowWhole;
    Assert(RowRemainder <= 1.0f);

    real32 ColumnMod = ModReal32(Column, (real32)RANDOM_TEX_SIZE);
    uint32 ColumnWhole = FloorInt32(ColumnMod);
    uint32 ColumnWhole2 = (ColumnWhole + 1) % RANDOM_TEX_SIZE;
    real32 ColumnRemainder = ColumnMod - (real32)ColumnWhole;
    Assert(ColumnRemainder <= 1.0f);
    
    v3 G00 = Generator->GradientTex[RowWhole*RANDOM_TEX_SIZE + ColumnWhole];
    v3 G01 = Generator->GradientTex[RowWhole*RANDOM_TEX_SIZE + ColumnWhole2];
    v3 G10 = Generator->GradientTex[RowWhole2*RANDOM_TEX_SIZE + ColumnWhole];
    v3 G11 = Generator->GradientTex[RowWhole2*RANDOM_TEX_SIZE + ColumnWhole2];
    
    v3 D00 = {RowRemainder, ColumnRemainder};
    v3 D01 = {RowRemainder, ColumnRemainder-1.0f };
    v3 D10 = {RowRemainder-1.0f, ColumnRemainder };
    v3 D11 = {RowRemainder-1.0f, ColumnRemainder - 1.0f };
    
    real32 R00 = DotProduct(G00, D00);
    real32 R01 = DotProduct(G01, D01);
    real32 R10 = DotProduct(G10, D10);
    real32 R11 = DotProduct(G11, D11);
    
    real32 I0 = Lerp(R00, R01, ColumnRemainder);
    real32 I1 = Lerp(R10, R11, ColumnRemainder);
    
    real32 Result = Lerp(I0, I1, RowRemainder) / 1.05f;
    Assert(Result >= -1.0f && Result <= 1.0f);
    return Result;
}

internal real32 
RandomFloat(perlin_noise_generator *Generator, real32 Plane, real32 Row, real32 Column)
{
    real32 PlaneMod = ModReal32(Plane, (real32)RANDOM_TEX_SIZE);
    uint32 PlaneWhole = FloorInt32(PlaneMod);
    uint32 PlaneWhole2 = (PlaneWhole + 1) % RANDOM_TEX_SIZE;
    real32 PlaneRemainder = PlaneMod - (real32)PlaneWhole;
    Assert(PlaneRemainder <= 1.0f);
    
    real32 RowMod = ModReal32(Row, (real32)RANDOM_TEX_SIZE);
    uint32 RowWhole = FloorInt32(RowMod);
    uint32 RowWhole2 = (RowWhole + 1) % RANDOM_TEX_SIZE;
    real32 RowRemainder = RowMod - (real32)RowWhole;
    Assert(RowRemainder <= 1.0f);

    real32 ColumnMod = ModReal32(Column, (real32)RANDOM_TEX_SIZE);
    uint32 ColumnWhole = FloorInt32(ColumnMod);
    uint32 ColumnWhole2 = (ColumnWhole + 1) % RANDOM_TEX_SIZE;
    real32 ColumnRemainder = ColumnMod - (real32)ColumnWhole;
    Assert(ColumnRemainder <= 1.0f);

    v3 G000 = Generator->GradientTex[PlaneWhole*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE + RowWhole*RANDOM_TEX_SIZE + ColumnWhole];
    v3 G001 = Generator->GradientTex[PlaneWhole*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE + RowWhole*RANDOM_TEX_SIZE + ColumnWhole2];
    v3 G010 = Generator->GradientTex[PlaneWhole*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE + RowWhole2*RANDOM_TEX_SIZE + ColumnWhole];
    v3 G011 = Generator->GradientTex[PlaneWhole*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE + RowWhole2*RANDOM_TEX_SIZE + ColumnWhole2];
    v3 G100 = Generator->GradientTex[PlaneWhole2*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE + RowWhole*RANDOM_TEX_SIZE + ColumnWhole];
    v3 G101 = Generator->GradientTex[PlaneWhole2*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE + RowWhole*RANDOM_TEX_SIZE + ColumnWhole2];
    v3 G110 = Generator->GradientTex[PlaneWhole2*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE + RowWhole2*RANDOM_TEX_SIZE + ColumnWhole];
    v3 G111 = Generator->GradientTex[PlaneWhole2*RANDOM_TEX_SIZE*RANDOM_TEX_SIZE + RowWhole2*RANDOM_TEX_SIZE + ColumnWhole2];

    v3 D000 = {PlaneRemainder, RowRemainder, ColumnRemainder};
    v3 D001 = {PlaneRemainder, RowRemainder, ColumnRemainder-1.0f };
    v3 D010 = {PlaneRemainder, RowRemainder-1.0f, ColumnRemainder };
    v3 D011 = {PlaneRemainder, RowRemainder-1.0f, ColumnRemainder - 1.0f };
    v3 D100 = {PlaneRemainder - 1.0f, RowRemainder, ColumnRemainder};
    v3 D101 = {PlaneRemainder - 1.0f, RowRemainder, ColumnRemainder-1.0f };
    v3 D110 = {PlaneRemainder - 1.0f, RowRemainder-1.0f, ColumnRemainder };
    v3 D111 = {PlaneRemainder - 1.0f, RowRemainder-1.0f, ColumnRemainder - 1.0f };

    real32 R000 = DotProduct(G000, D000);
    real32 R001 = DotProduct(G001, D001);
    real32 R010 = DotProduct(G010, D010);
    real32 R011 = DotProduct(G011, D011);
    real32 R100 = DotProduct(G100, D100);
    real32 R101 = DotProduct(G101, D101);
    real32 R110 = DotProduct(G110, D110);
    real32 R111 = DotProduct(G111, D111);

    real32 I00 = Lerp(R000, R001, ColumnRemainder);
    real32 I01 = Lerp(R010, R011, ColumnRemainder);
    real32 I10 = Lerp(R100, R101, ColumnRemainder);
    real32 I11 = Lerp(R110, R111, ColumnRemainder);

    real32 I0 = Lerp(I00, I01, RowRemainder);
    real32 I1 = Lerp(I10, I11, RowRemainder);
    
    real32 Result = Lerp(I0, I1, PlaneRemainder) / 1.05f;
    Assert(Result >= -1.0f && Result <= 1.0f);
    return Result;
}

internal real32 
RandomFloat(perlin_noise_generator *Generator, v3 WorldPos)
{
    real32 Result = RandomFloat(Generator, WorldPos.X, WorldPos.Y, WorldPos.Z);
    return Result;
}