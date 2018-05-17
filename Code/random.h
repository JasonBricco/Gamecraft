// Voxel Engine
// Jason Bricco

// Returns a random number between 0 and max.
inline int32_t Rand(int32_t max)
{
    return rand() % max;
}

// Returns a random number between 0 and 1.
inline float Rand01()
{
    float divisor = 1.0f / RAND_MAX;
    return divisor * rand();
}

// Returns a random number between -1 and 1.
inline float RandNormal()
{
    return (Rand01() * 2.0f) - 1.0f;
}

// Returns a random number between min and max.
inline float RandRange(float min, float max)
{
    float range = max - min;
    return (Rand01() * range) + min;
}

// Returns a random number between min and max.
inline int RandRange(int min, int max)
{
    return min + rand() % ((max + 1) - min);
}
