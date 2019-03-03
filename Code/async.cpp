//
// Jason Bricco
//

static inline bool DoNextAsync(GameState* state, AsyncWorkQueue& queue)
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

            if (item.callback != nullptr)
            {
            	WaitForSingleObject(state->callbackMutex, INFINITE);
			    AsyncCallbackItem cb = { item.callback, item.world, item.data };
			    state->callbacks.push_back(cb);
			    ReleaseMutex(state->callbackMutex);
            }
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
		if (DoNextAsync(state, state->workQueue)) 
			WaitForSingleObject(state->semaphore, INFINITE);
    }
}

static void RunAsyncCallbacks(GameState* state)
{
	WaitForSingleObject(state->callbackMutex, INFINITE);

	for (int i = 0; i < state->callbacks.size(); i++)
	{
		AsyncCallbackItem item = state->callbacks[i];
		item.callback(item.world, item.data);
	}

	state->callbacks.clear();
	ReleaseMutex(state->callbackMutex);
}

#if MULTITHREADING

static inline void QueueAsync(GameState* state, AsyncFunc func, World* world, void* data, AsyncCallback callback = nullptr)
{
	AsyncWorkQueue& asyncQueue = state->workQueue;
	uint32_t nextWrite = (asyncQueue.write + 1) & (asyncQueue.size - 1);
	assert(nextWrite != asyncQueue.read);
	AsyncItem* asyncItem = asyncQueue.items + asyncQueue.write;
	asyncItem->func = func;
	asyncItem->world = world;
	asyncItem->data = data;
	asyncItem->callback = callback;
	asyncQueue.write = nextWrite;
	ReleaseSemaphore(state->semaphore, 1, NULL);
}

#else

static inline void QueueAsync(GameState*, AsyncFunc func, World* world, void* data, AsyncCallback callback = nullptr)
{
	func(world, data);

	if (callback != nullptr)
		callback(world, data);
}

#endif

static void CreateThreads(GameState* state)
{
	state->semaphore = CreateSemaphore(NULL, 0, MAXLONG, NULL);

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	int threadCount = info.dwNumberOfProcessors - 1;
	Print("Creating %i threads.\n", threadCount)

	AsyncWorkQueue& asyncQueue = state->workQueue;
	asyncQueue.size = 2048;
	asyncQueue.items = AllocArray(asyncQueue.size, AsyncItem);

	state->callbackMutex = CreateMutex(NULL, FALSE, NULL);

	for (int i = 0; i < threadCount; i++)
	{
		DWORD threadID; 
		HANDLE handle = CreateThread(NULL, NULL, ThreadProc, state, NULL, &threadID);
        CloseHandle(handle);
	}
}
