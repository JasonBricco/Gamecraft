// Voxel Engine
// Jason Bricco

#if PROFILING || PROFILING_ONCE

enum MeasureSection
{
	MEASURE_GAME_LOOP = 0,
	MEASURE_RENDER_SCENE = 1,
	MEASURE_PLAYER_COLLISION = 2,
	MEASURE_BUILD_CHUNK = 3,
	MEASURE_GEN_TERRAIN = 4,
	MEASURE_TEMP = 5,
	MEASURE_COUNT = 6
};

struct CycleCounter
{
	uint64_t cycles;
	uint64_t calls;
};

static CycleCounter g_counters[MEASURE_COUNT];

static void FlushCounters()
{
	OutputDebugString("CYCLE COUNTS:\n");

	for (int i = 0; i < MEASURE_COUNT; i++)
	{
		uint64_t calls = g_counters[i].calls;

		if (calls > 0)
		{
			uint64_t cycles = g_counters[i].cycles;
			uint64_t cyclesPerCall = cycles / calls;
			char buffer[128];
			sprintf(buffer, "%d: Cycles: %I64u, Calls: %I64u, Cycles/Call: %I64u\n", 
				i, cycles, calls, cyclesPerCall);
			OutputDebugString(buffer);

			g_counters[i].cycles = 0;
			g_counters[i].calls = 0;
		}
	}
}

inline void EndTimedBlock(int ID, uint64_t start)
{
	g_counters[ID].cycles += __rdtsc() - start;
	g_counters[ID].calls++;
}

inline void EndTimedBlockDirect(int ID, uint64_t start)
{
	EndTimedBlock(ID, start);
	FlushCounters();
}

#define BEGIN_TIMED_BLOCK(ID) uint64_t startCount##ID = __rdtsc();
#define END_TIMED_BLOCK(ID) EndTimedBlock(MEASURE_##ID, startCount##ID)

#else

#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)

#endif

#if PROFILING
#define FLUSH_COUNTERS() FlushCounters();
#else 
#define FLUSH_COUNTERS()
#endif

#if PROFILING_ONCE
#define END_TIMED_BLOCK_DIRECT(ID) EndTimedBlockDirect(MEASURE_##ID, startCount##ID);
#else
#define END_TIMED_BLOCK_DIRECT(ID)
#endif
