//
// Gamecraft
//

using AsyncCallback = void(*)(GameState* state, World*, void*);
using AsyncFunc = void(*)(GameState* state, World*, void*);

struct AsyncItem
{
    AsyncFunc func;
    GameState* state;
    World* world;
    void* data;
    AsyncCallback callback;
};

struct AsyncCallbackItem
{
    AsyncCallback callback;
    World* world;
    void* data;
};

struct AsyncWorkQueue
{
    // Read and write mark the locations where we write values into and read values 
    // from the queue. Write should write ahead while read stays behind. If read 
    // catches up to write, then it has run out of work to do. If write catches up to 
    // read, then we will overwrite work in the queue that hasn't been completed yet.
    uint32_t read;
    uint32_t write;

    // Stores all work to be done by background threads.
    AsyncItem* items;
    int size;
};
