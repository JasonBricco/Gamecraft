// Voxel Engine
// Jason Bricco
// Created March 23, 2018

struct World;
struct Chunk;

inline void DisplayError(char* message);
static char* PathToAsset(char* fileName);

inline bool BlockInsideWorld(World* world, int x, int y, int z);
static Chunk* CreateChunk(World* world, int x, int z);
