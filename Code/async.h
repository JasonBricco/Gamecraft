// Voxel Engine
// Jason Bricco

#define THREAD_COUNT 7

using AsyncFunc = void(*)(void*);

struct AsyncItem
{
    AsyncFunc func;
    void* data;
};

struct ThreadWorkQueue
{
    // Read and write mark the locations where we write values into and read values 
    // from the queue. Write should write ahead, while read stays behind. If read 
    // catches up to write, then it has run out of work to do. If write catches up to 
    // read, then we will overwrite work in the queue that hasn't been completed yet.
    atomic<uint32_t> read;
    atomic<uint32_t> write;

    atomic<uint32_t> target;
    atomic<uint32_t> completed;

    // Stores all work to be done by background threads.
    AsyncItem* items;
    int size;
};

inline void SemaphoreWait();
inline void SemaphoreSignal(int32_t count);
inline bool DoNextAsync();
static void QueueAsync(AsyncFunc func, void* data);
static void CreateThreads();
