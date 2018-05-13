// Voxel Engine
// Jason Bricco

// Holds work to be added by the main thread and performed by background threads.
static WorkQueue g_workQueue;

static HANDLE g_semaphore;

inline void SemaphoreWait()
{
	WaitForSingleObject(g_semaphore, INFINITE);
}

inline void SemaphoreSignal(int32_t count)
{
	ReleaseSemaphore(g_semaphore, count, NULL);
}

inline bool DoNextAsync()
{
    bool sleep = false;
 
    uint32_t originalRead = g_workQueue.read;
    uint32_t nextRead = (originalRead + 1) & (g_workQueue.size - 1);
 
    // If read and write are equal, then we have run out of work to do. Write is the location
    // where the next item will be placed - it isn't placed yet.
    if (originalRead != g_workQueue.write)
    {
        // Increments the work queue's next value only if it matches our expected next value. 
        // This prevents problems caused by multiple threads running this code.
        if (atomic_compare_exchange_weak(&g_workQueue.read, &originalRead, nextRead))
        {
            AsyncItem item = g_workQueue.items[originalRead];
            item.func(item.world, item.chunk);
        }
    }
    else sleep = true;
 
    return sleep;
}

static DWORD WINAPI ThreadProc(LPVOID param)
{
	while (true)
	{
		if (DoNextAsync()) 
			SemaphoreWait();
    }
}

inline void QueueAsync(AsyncFunc func, World* world, Chunk* chunk)
{
	uint32_t nextWrite = (g_workQueue.write + 1) & (g_workQueue.size - 1);
	Assert(nextWrite != g_workQueue.read);
	AsyncItem* item = g_workQueue.items + g_workQueue.write;
	item->func = func;
	item->world = world;
	item->chunk = chunk;
	g_workQueue.write = nextWrite;
	SemaphoreSignal(1);
}

static void CreateThreads()
{
	g_semaphore = CreateSemaphore(NULL, 0, MAXLONG, NULL);

	g_workQueue.size = 4096;
	g_workQueue.items = Calloc(AsyncItem, g_workQueue.size * sizeof(AsyncItem));

	for (int i = 0; i < THREAD_COUNT; i++)
	{
		DWORD threadID; 
		HANDLE handle = CreateThread(NULL, NULL, ThreadProc, NULL, NULL, &threadID);
        CloseHandle(handle);
	}
}
