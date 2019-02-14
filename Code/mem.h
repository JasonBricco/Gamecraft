//
// Jason Bricco
//

struct TempMemory
{
	uint32_t size, used;
	uint8_t* data;
};

static TempMemory g_memory;

static inline uint8_t* _PushMemory(uint32_t size)
{
	assert(g_memory.used + size < g_memory.size);
	uint8_t* ptr = g_memory.data + g_memory.used;
	g_memory.used += size;
	return ptr;
}

static inline void WipeTempMemory()
{
	g_memory.used = 0;
}

#define AllocTempStruct(item) (item*)_PushMemory(sizeof(item))
#define AllocTempArray(count, item) (item*)_PushMemory(count * sizeof(item))
#define AllocTempRaw(size, item) (item*)_PushMemory(size)

#define Construct(ptr, item) new (ptr)item();

#if DEBUG_MEMORY

struct DebugAllocInfo
{
	char* str;
	int count;
};

static vector<DebugAllocInfo> g_allocs;
static HANDLE g_allocMutex;

static char* GetAllocLocStr(char* file, int line)
{
	char name[_MAX_FNAME];
	_splitpath(file, NULL, NULL, name, NULL);

	char* str = (char*)calloc(1, strlen(file) + sizeof(int));
	sprintf(str, "%s %i", name, line);

	return str;
}

static void TrackAlloc(char* str)
{
	WaitForSingleObject(g_allocMutex, INFINITE);

	bool found = false;

	for (int i = 0; i < g_allocs.size(); i++)
	{
		DebugAllocInfo& info = g_allocs[i];

		if (strcmp(info.str, str) == 0)
		{
			info.count++;
			found = true;
		}
	}

	if (!found)
	{
		DebugAllocInfo info = { str, 1 };
		g_allocs.push_back(info);
	}

	ReleaseMutex(g_allocMutex);
}

static void UntrackAlloc(char* str)
{
	WaitForSingleObject(g_allocMutex, INFINITE);

	bool found = false;

	for (auto it = g_allocs.begin(); it != g_allocs.end(); it++)
	{
		if (strcmp(it->str, str) == 0)
		{
			found = true;
			it->count--;

			if (it->count == 0)
			{
				free(it->str);
				g_allocs.erase(it);
				break;
			}
		}
	}

	if (!found)
		Error("Allocation for %s was not in the alloc table.\n", str);

	ReleaseMutex(g_allocMutex);
}

struct DebugAllocHeader
{
	char* str;
};

static void* DebugMalloc(int size, char* file, int line)
{
	char* str = GetAllocLocStr(file, line);
	TrackAlloc(str);
	uint8_t* data = (uint8_t*)malloc(size + sizeof(DebugAllocHeader));
	DebugAllocHeader* header = (DebugAllocHeader*)data;
	header->str = str;
	return (void*)(data + sizeof(DebugAllocHeader));
}

static void* DebugCalloc(int count, int size, char* file, int line)
{
	void* data = DebugMalloc(count * size, file, line);
	memset(data, 0, count * size);
	return data;
}

static void DebugFree(void* ptr)
{
	uint8_t* head = (uint8_t*)ptr;
	DebugAllocHeader* header = (DebugAllocHeader*)(head - sizeof(DebugAllocHeader));
	UntrackAlloc(header->str);
	free(head - sizeof(DebugAllocHeader));
}

static void DumpMemoryInfo()
{
	Print("ALLOCATION REPORT: \n\n");

	for (auto it = g_allocs.begin(); it != g_allocs.end(); it++)
		Print("%s: %i\n", it->str, it->count);
}

#define AllocStruct(type) (type*)DebugMalloc(sizeof(type), __FILE__, __LINE__)
#define CallocStruct(type) (type*)DebugCalloc(1, sizeof(type), __FILE__, __LINE__)

#define AllocArray(count, type) (type*)DebugMalloc(count * sizeof(type), __FILE__, __LINE__)
#define CallocArray(count, type) (type*)DebugCalloc(count, sizeof(type), __FILE__, __LINE__)

#define AllocRaw(size) DebugMalloc(size, __FILE__, __LINE__)

#define Free(ptr) DebugFree(ptr)

#else

#define AllocStruct(type) (type*)malloc(sizeof(type))
#define CallocStruct(type) (type*)calloc(1, sizeof(type))

#define AllocArray(count, type) (type*)malloc(count * sizeof(type))
#define CallocArray(count, type) (type*)calloc(count, sizeof(type))

#define AllocRaw(size) malloc(size)

#define Free(ptr) free(ptr)

#endif

#define ReallocArray(ptr, count, type) (type*)realloc(ptr, count * sizeof(type))
#define ReallocRaw(ptr, size) realloc(ptr, size)
