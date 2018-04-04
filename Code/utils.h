// Voxel Engine
// Jason Bricco
// Created March 28, 2018

#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

inline int Clamp(int value, int min, int max)
{
	return value <= min ? min : value >= max ? max : value;
}

inline int Square(int value)
{
	return value * value;
}
