// Voxel Engine
// Jason Bricco

#if PROFILING

enum MeasureSection
{
	MEASURE_GAME_LOOP = 0,
	MEASURE_RENDER_SCENE = 1,
	MEASURE_BUILD_CHUNK = 2,
	MEASURE_GEN_TERRAIN = 3,
	MEASURE_TEMP = 4,
	MEASURE_COUNT = 5
};

struct CycleCounter
{
	uint64_t cycles;
	uint64_t calls;
	uint64_t lowest = UINT_MAX;
};

static CycleCounter g_counters[MEASURE_COUNT];

static void FlushCounters()
{
	static bool profilingEnabled = true;

	if (profilingEnabled)
	{
		OutputDebugString("CYCLE COUNTS:\n");

		for (int i = 0; i < MEASURE_COUNT; i++)
		{
			uint64_t calls = g_counters[i].calls;

			if (calls > 0)
			{
				uint64_t cycles = g_counters[i].cycles;
				uint64_t cyclesPerCall = cycles / calls;

				if (cycles < g_counters[i].lowest)
					g_counters[i].lowest = cycles;

				char buffer[128];
				sprintf(buffer, "%d: Cycles: %I64u, Calls: %I64u, Cycles/Call: %I64u, Lowest: %I64u\n", 
					i, cycles, calls, cyclesPerCall, g_counters[i].lowest);
				OutputDebugString(buffer);

				g_counters[i].cycles = 0;
				g_counters[i].calls = 0;
			}
		}
	}

	#if PROFILING_ONCE

	profilingEnabled = false;

	#endif
}

inline void EndTimedBlock(int ID, uint64_t start)
{
	g_counters[ID].cycles += __rdtsc() - start;
	g_counters[ID].calls++;
}

#define BEGIN_TIMED_BLOCK(ID) uint64_t startCount##ID = __rdtsc();
#define END_TIMED_BLOCK(ID) EndTimedBlock(MEASURE_##ID, startCount##ID)
#define FLUSH_COUNTERS() FlushCounters();

#else

#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#define FLUSH_COUNTERS()

#endif
