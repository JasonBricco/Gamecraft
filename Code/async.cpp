//
// Jason Bricco
//

static inline bool DoNextAsync(WorkQueue& queue)
{
    bool sleep = false;
 
    uint32_t originalRead = queue.read;
    uint32_t nextRead = (originalRead + 1) & (queue.size - 1);

    // If read and write are equal, then we have run out of work to do. Write is the location
    // where the next item will be placed - it isn't placed yet.
    if (originalRead != queue.write)
    {
    	// Increments the work queue's next value only if it matches our expected next value. 
        // This prevents problems caused by multiple threads running this code.
    	if (InterlockedCompareExchange(&queue.read, nextRead, originalRead) == originalRead)
	    {
	    	AsyncItem item = queue.items[originalRead];
            item.func(item.world, item.data);
	    }
    }
 	else sleep = true;
 
    return sleep;
}

static DWORD WINAPI ThreadProc(LPVOID ptr)
{
	GameState* state = (GameState*)ptr;

	while (true)
	{
		if (DoNextAsync(state->workQueue)) 
			WaitForSingleObject(state->semaphore, INFINITE);
    }
}

#if MULTITHREADING

static inline void QueueAsync(GameState* state, AsyncFunc func, World* world, void* data, AsyncCallback callback = nullptr)
{
	WorkQueue& queue = state->workQueue;
	uint32_t nextWrite = (queue.write + 1) & (queue.size - 1);
	assert(nextWrite != queue.read);
	AsyncItem* item = queue.items + queue.write;
	item->func = func;
	item->world = world;
	item->data = data;
	item->callback = callback;
	queue.write = nextWrite;
	ReleaseSemaphore(state->semaphore, 1, NULL);
}

#else

static inline void QueueAsync(GameState*, AsyncFunc func, World* world, void* data, AsyncCallback callback = nullptr)
{
	Unused(callback);
	func(world, data);
}

#endif

static void CreateThreads(GameState* state)
{
	state->semaphore = CreateSemaphore(NULL, 0, MAXLONG, NULL);

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	int threadCount = info.dwNumberOfProcessors - 1;
	Print("Creating %i threads.\n", threadCount)

	WorkQueue& queue = state->workQueue;
	queue.size = 4096;
	queue.items = AllocArray(queue.size, AsyncItem);

	for (int i = 0; i < threadCount; i++)
	{
		DWORD threadID; 
		HANDLE handle = CreateThread(NULL, NULL, ThreadProc, state, NULL, &threadID);
        CloseHandle(handle);
	}
}
