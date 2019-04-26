//
// Gamecraft
//

struct Memory
{
	uint8_t* data;
	uint32_t size, used;

	uint8_t* tempData;
	uint32_t tempSize, tempUsed;

	CRITICAL_SECTION cs;
	bool csValid;
};

static Memory g_memory;

static inline void MemoryBlockThreads()
{
	if (!g_memory.csValid)
	{
		InitializeCriticalSection(&g_memory.cs);
		g_memory.csValid = true;
	}

	EnterCriticalSection(&g_memory.cs);
}

static inline void MemoryUnblockThreads()
{
	LeaveCriticalSection(&g_memory.cs);
}

#if DEBUG_MEMORY
#pragma message("Memory debugging enabled.")

// Debug alloc: protect each side of the requested memory with PAGE_NOACCESS to
// cause crashes when writing outside the bounds of memory.
static inline uint8_t* _DebugPushMemory(uint32_t requested)
{
	uint32_t size = Align4096(requested + 2 * 4096);

	uint8_t* addr = (uint8_t*)VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
	addr += 4096;

	return (uint8_t*)VirtualAlloc(addr, requested, MEM_COMMIT, PAGE_READWRITE);
}

#define AllocStruct(item) (item*)_DebugPushMemory(sizeof(item))
#define AllocArray(count, item) (item*)_DebugPushMemory(count * sizeof(item))
#define AllocRaw(size) _DebugPushMemory(size)

#else

// Alloc: allocate memory to a fixed block created on program startup.
static inline uint8_t* _PushMemory(uint32_t size)
{
	MemoryBlockThreads();
	assert(g_memory.used + size < g_memory.size);
	uint8_t* ptr = g_memory.data + g_memory.used;
	g_memory.used += size;
	MemoryUnblockThreads();
	return ptr;
}

#define AllocStruct(item) (item*)_PushMemory(sizeof(item))
#define AllocArray(count, item) (item*)_PushMemory(count * sizeof(item))
#define AllocRaw(size) _PushMemory(size)

#endif

static inline uint8_t* _PushTempMemory(uint32_t size)
{
	assert(g_memory.tempUsed + size < g_memory.tempSize);
	uint8_t* ptr = g_memory.tempData + g_memory.tempUsed;
	g_memory.tempUsed += size;
	return ptr;
}

static inline void WipeTempMemory()
{
	g_memory.tempUsed = 0;
}

#define AllocTempStruct(item) (item*)_PushTempMemory(sizeof(item))
#define AllocTempArray(count, item) (item*)_PushTempMemory(count * sizeof(item))
#define AllocTempRaw(size) (item*)_PushTempMemory(size)

#define Construct(ptr, item) new (ptr)item();

template <typename T>
struct ObjectPool
{
	int size, free, capacity;	
	T** items;

	T* Get()
	{
		T* item = size == 0 ? AllocStruct(T) : items[--size];
		free++;
		assert(free <= capacity - size);
		return item;
	}
	
	void Return(T* item)
	{
		items[size++] = item;
		free--;
		assert(size <= capacity);
	}
};

template <typename T>
static ObjectPool<T> CreatePool(int capacity)
{
	ObjectPool<T> pool = { 0, 0, capacity };
	pool.items = AllocArray(capacity, T*);
	return pool;
}
