//
// Gamecraft
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
            item.func(item.state, item.world, item.data);

            if (item.callback != nullptr)
            {
            	AcquireSRWLockExclusive(&state->callbackLock);
			    AsyncCallbackItem cb = { item.callback, item.world, item.data };
			    state->callbacks.push_back(cb);
			    ReleaseSRWLockExclusive(&state->callbackLock);
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
	AcquireSRWLockExclusive(&state->callbackLock);

	for (int i = 0; i < state->callbacks.size(); i++)
	{
		AsyncCallbackItem item = state->callbacks[i];
		item.callback(item.world, item.data);
	}

	state->callbacks.clear();
	ReleaseSRWLockExclusive(&state->callbackLock);
}

static inline void QueueAsync(GameState* state, AsyncFunc func, World* world, void* data, AsyncCallback callback = nullptr)
{
	AsyncWorkQueue& asyncQueue = state->workQueue;
	uint32_t nextWrite = (asyncQueue.write + 1) & (asyncQueue.size - 1);
	assert(nextWrite != asyncQueue.read);
	AsyncItem* asyncItem = asyncQueue.items + asyncQueue.write;
	asyncItem->func = func;
	asyncItem->state = state;
	asyncItem->world = world;
	asyncItem->data = data;
	asyncItem->callback = callback;
	asyncQueue.write = nextWrite;
	ReleaseSemaphore(state->semaphore, 1, NULL);
}

static int CreateThreads(GameState* state)
{
	state->semaphore = CreateSemaphore(NULL, 0, MAXLONG, NULL);

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	int threadCount = info.dwNumberOfProcessors - 1;

	AsyncWorkQueue& asyncQueue = state->workQueue;
	asyncQueue.size = 2048;
	asyncQueue.items = new AsyncItem[asyncQueue.size];

	state->callbacks.reserve(1024);
	InitializeSRWLock(&state->callbackLock);

	for (int i = 0; i < threadCount; i++)
	{
		DWORD threadID; 
		HANDLE handle = CreateThread(NULL, NULL, ThreadProc, state, NULL, &threadID);
        CloseHandle(handle);
	}

	return threadCount;
}
