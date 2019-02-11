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
