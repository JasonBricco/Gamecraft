// Voxel Engine
// Jason Bricco

#define THREAD_COUNT 7

struct ThreadInfo
{
	int index;
};

struct WorkItem
{
	char* str;	
};

static void CreateThreads();
