//
// Jason Bricco
// 

#if DEBUG_MEMORY

static unordered_map<string, uint32_t> allocInfo;
static mutex allocMutex;

static void DebugTrackAlloc(string id)
{
    allocMutex.lock();
    auto it = allocInfo.find(id);

    if (it == allocInfo.end())
        allocInfo[id] = 0;

    allocInfo[id]++;
    allocMutex.unlock();
}

static void DebugUntrackAlloc(string id)
{
    allocMutex.lock();
    auto it = allocInfo.find(id);
    assert(it != allocInfo.end());

    allocInfo[id]--;
    allocMutex.unlock();
}

static void LogMemoryStatus()
{
    string str;
    str.append("ALLOC INFO:\n");

    for (pair<string, uint32_t> element : allocInfo)
    {
        if (element.second > 0)
        {
            str.append("ID: " + element.first + "\n");
            str.append("Count: " + to_string(element.second) + "\n");
        }
    }

    str.append("\n");
    OutputDebugString(str.c_str());
}

#define TrackAlloc(id) DebugTrackAlloc(typeid(id).name())
#define UntrackAlloc(id) DebugUntrackAlloc(typeid(id).name())

#else

#define TrackAlloc(id)
#define UntrackAlloc(id)

#endif

template <class T, typename... args>
static inline T* Calloc()
{
    TrackAlloc(T);
    T* data = (T*)calloc(1, sizeof(T));
    return new (data)T(args...);
}

template <class T, typename... args>
static inline T* Malloc()
{
    TrackAlloc(T);
    T* data = (T*)malloc(sizeof(T));
    return new (data)T(args...);
}

template <class T>
static inline T* Malloc(int count)
{
    TrackAlloc(T);
    return (T*)malloc(count * sizeof(T));
}

template <class T>
static inline T* Calloc(int count)
{
    TrackAlloc(T);
    return (T*)calloc(count, sizeof(T));
}

template <class T>
static inline T* Realloc(void* ptr, int count)
{
    return (T*)realloc(ptr, count * sizeof(T));
}

template <class T>
static inline T* AlignedMalloc(int count, int alignment)
{
    TrackAlloc(T);
    return (T*)_aligned_malloc(count * sizeof(T), alignment);
}

template <class T>
static inline void Free(void* ptr)
{
    UntrackAlloc(T);
    free(ptr);
}
