//
// Gamecraft
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
	char* type;
	int count;
	float sizeInMb;
};

struct DebugAllocHeader
{
	char* str;
};

static vector<DebugAllocInfo> g_allocs;
static int g_glAlloc = 0;
static HANDLE g_allocMutex = NULL;

typedef vector<DebugAllocInfo>::iterator DebugAllocIterator;

static char* GetAllocLocStr(char* file, int line)
{
	char name[_MAX_FNAME];
	_splitpath(file, NULL, NULL, name, NULL);

	char* str = (char*)calloc(1, strlen(file) + sizeof(int));
	sprintf(str, "%s %i", name, line);

	return str;
}

static DebugAllocIterator GetAllocInfo(char* str)
{
	for (auto it = g_allocs.begin(); it != g_allocs.end(); it++)
	{
		if (strcmp(it->str, str) == 0)
			return it;
	}

	return g_allocs.end();
}

static void TrackAlloc(char* str, char* type, float sizeInMb)
{
	DebugAllocIterator it = GetAllocInfo(str);

	if (it == g_allocs.end())
	{
		DebugAllocInfo newInfo = { str, type, 1 , sizeInMb };
		g_allocs.push_back(newInfo);
	}
	else
	{
		it->count++;
		it->sizeInMb += sizeInMb;
	}
}

static void UntrackAlloc(char* str)
{
	DebugAllocIterator it = GetAllocInfo(str);
	assert(it != g_allocs.end());

	it->count--;
	it->sizeInMb -= (it->sizeInMb / it->count);

	if (it->count == 0)
	{
		free(it->str);
		g_allocs.erase(it);
	}
}

static inline void AllocMutexLock()
{
	if (g_allocMutex == NULL)
		g_allocMutex = CreateMutex(NULL, FALSE, NULL);

	WaitForSingleObject(g_allocMutex, INFINITE);
}

static void* DebugMalloc(int size, char* file, int line, char* type)
{
	AllocMutexLock();

	char* str = GetAllocLocStr(file, line);
	TrackAlloc(str, type, size / 1048576.0f);
	uint8_t* data = (uint8_t*)malloc(size + sizeof(DebugAllocHeader));
	DebugAllocHeader* header = (DebugAllocHeader*)data;
	header->str = str;
	void* result = (void*)(data + sizeof(DebugAllocHeader));

	ReleaseMutex(g_allocMutex);

	return result;
}

static void* DebugCalloc(int count, int size, char* file, int line, char* type)
{
	void* data = DebugMalloc(count * size, file, line, type);
	memset(data, 0, count * size);
	return data;
}

static void* DebugRealloc(void* ptr, int size, char* file, int line, char* type)
{
	if (ptr == nullptr)
		return DebugMalloc(size, file, line, type);
	else
	{
		AllocMutexLock();

		uint8_t* head = (uint8_t*)ptr;
		DebugAllocHeader* header = (DebugAllocHeader*)(head - sizeof(DebugAllocHeader));
		DebugAllocIterator it = GetAllocInfo(header->str);

		float avgPerItem = it->sizeInMb / it->count;
		it->sizeInMb += (avgPerItem * 2.0f);

		head = (uint8_t*)realloc(head - sizeof(DebugAllocHeader), size);
		void* result = (void*)(head + sizeof(DebugAllocHeader));

		ReleaseMutex(g_allocMutex);

		return result;
	}
}

static void DebugFree(void* ptr)
{
	AllocMutexLock();

	uint8_t* head = (uint8_t*)ptr;
	DebugAllocHeader* header = (DebugAllocHeader*)(head - sizeof(DebugAllocHeader));
	UntrackAlloc(header->str);
	free(head - sizeof(DebugAllocHeader));

	ReleaseMutex(g_allocMutex);
}

static void DumpMemoryInfo()
{
	Print("ALLOCATION REPORT: \n\n");

	for (auto it = g_allocs.begin(); it != g_allocs.end(); it++)
		Print("%s (%s): %i [%.2f mb]\n", it->str, it->type, it->count, it->sizeInMb);

	Print("GL Allocs: %i\n", g_glAlloc);
	Print("\n");
}

#define AllocStruct(type) (type*)DebugMalloc(sizeof(type), __FILE__, __LINE__, #type)
#define CallocStruct(type) (type*)DebugCalloc(1, sizeof(type), __FILE__, __LINE__, #type)

#define AllocArray(count, type) (type*)DebugMalloc(count * sizeof(type), __FILE__, __LINE__, #type)
#define CallocArray(count, type) (type*)DebugCalloc(count, sizeof(type), __FILE__, __LINE__, #type)

#define AllocRaw(size) DebugMalloc(size, __FILE__, __LINE__, "Raw")

#define Free(ptr) DebugFree(ptr)

#define ReallocArray(ptr, count, type) (type*)DebugRealloc(ptr, count * sizeof(type), __FILE__, __LINE__, #type)
#define ReallocRaw(ptr, size) DebugRealloc(ptr, size, __FILE__, __LINE__, "Raw")

#define TrackGLAllocs(count) g_glAlloc += count
#define UntrackGLAllocs(count) g_glAlloc -= count

#else

#define AllocStruct(type) (type*)malloc(sizeof(type))
#define CallocStruct(type) (type*)calloc(1, sizeof(type))

#define AllocArray(count, type) (type*)malloc(count * sizeof(type))
#define CallocArray(count, type) (type*)calloc(count, sizeof(type))

#define AllocRaw(size) malloc(size)

#define Free(ptr) free(ptr)

#define ReallocArray(ptr, count, type) (type*)realloc(ptr, count * sizeof(type))
#define ReallocRaw(ptr, size) realloc(ptr, size)

#define TrackGLAllocs(count)
#define UntrackGLAllocs(count)

#endif
