//
// Jason Bricco
//

struct Memory
{
	uint32_t size, used;
	uint32_t tempLoc, tempUsed;
	uint8_t* data;
};

static Memory g_memory;

static inline uint8_t* _PushMemory(uint32_t size)
{
	assert(g_memory.used + size < g_memory.tempLoc);
	uint8_t* ptr = g_memory.data + g_memory.used;
	g_memory.used += size;
	return ptr;
}

static inline uint8_t* _PushTemp(uint32_t size)
{
	assert(g_memory.tempUsed + size <= g_memory.size);
	uint8_t* ptr = g_memory.data + g_memory.tempUsed;
	g_memory.tempUsed += size;
	return ptr;
}

static inline void WipeTempMemory()
{
	g_memory.tempUsed = g_memory.tempLoc;
}

#define PushStruct(item) (item*)_PushMemory(sizeof(item))
#define Construct(ptr, item) new (ptr)item();

#define PushArray(count, item) (item*)_PushMemory(count * sizeof(item))

#define PushRaw(size, item) (item*)_PushMemory(size)

#define PushTempStruct(item) (item*)_PushTemp(sizeof(item))
#define PushTempArray(count, item) (item*)_PushTemp(count * sizeof(item))
