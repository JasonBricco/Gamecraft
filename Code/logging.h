// Voxel Engine
// Jason Bricco

inline void LogError(char* message)
{
	fprintf(stderr, "%s\n", message);
}

inline void PrintVec3(Vec3 v)
{
	printf("(%.02f, %.02f, %.02f)\n", v.x, v.y, v.z);
}
