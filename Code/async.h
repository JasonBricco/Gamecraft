// Voxel Engine
// Jason Bricco

#define THREAD_COUNT 3

using AsyncFunc = void(*)(World*, Chunk*);

struct AsyncItem
{
    AsyncFunc func;
    World* world;
    Chunk* chunk;
};

struct WorkQueue
{
    // Read and write mark the locations where we write values into and read values 
    // from the queue. Write should write ahead while read stays behind. If read 
    // catches up to write, then it has run out of work to do. If write catches up to 
    // read, then we will overwrite work in the queue that hasn't been completed yet.
    atomic<uint32_t> read;
    atomic<uint32_t> write;

    // Stores all work to be done by background threads.
    AsyncItem* items;
    int size;
};
