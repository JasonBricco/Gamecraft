// Voxel Engine
// Jason Bricco

#if 0
static queue<WorkItem> g_workQueue;

static DWORD WINAPI ThreadProc(LPVOID param)
{
	ThreadInfo* info = (ThreadInfo*)param;

	while (true)
	{
		if (g_workQueue.size > 0)
		{
			char* str = g_workQueue.pop();
			OutputDebugString(str);
		}
	}
}

static void QueueWork(char* string)
{
	WorkItem item = { string };
	g_workQueue.push(item);
}

static void CreateThreads()
{
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		ThreadInfo info = { i };

		DWORD threadID; 
		HANDLE thread = CreateThread(NULL, NULL, ThreadProc, &info, NULL, &threadID);
		CloseHandle(thread);
	}
}
#endif
