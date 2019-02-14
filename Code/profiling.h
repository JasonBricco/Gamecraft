//
// Jason Bricco
// 

#if PROFILING

enum MeasureSection
{
    MEASURE_GAME_LOOP,
    MEASURE_BUILD_CHUNK,
    MEASURE_CHUNK_GEN,
    MEASURE_FILL_MESH,
    MEASURE_COUNT
};

struct CycleCounter
{
    char* label;
    uint64_t cycles;
    uint64_t calls;
};

static CycleCounter g_counters[MEASURE_COUNT];

static void FlushCounters()
{
    static bool profilingEnabled = true;

    if (profilingEnabled)
    {
        bool doPrint = false;

        for (int i = 0; i < MEASURE_COUNT; i++)
        {
            if (g_counters[i].calls > 0)
                doPrint = true;
        }

        if (doPrint)
            OutputDebugString("CYCLE COUNTS:\n");
        else return;

        for (int i = 0; i < MEASURE_COUNT; i++)
        {
            uint64_t calls = g_counters[i].calls;

            if (calls > 0)
            {
                uint64_t cycles = g_counters[i].cycles;
                uint64_t cyclesPerCall = cycles / calls;

                char buffer[128];
                sprintf(buffer, "%s: Cycles: %I64u, Calls: %I64u, Cycles/Call: %I64u\n", g_counters[i].label, cycles, calls, cyclesPerCall);
                OutputDebugString(buffer);

                g_counters[i].cycles = 0;
                g_counters[i].calls = 0;
            }
        }

        OutputDebugString("\n");
    }
}

inline void EndTimedBlock(int ID, char* label, uint64_t start)
{
    g_counters[ID].label = label;
    g_counters[ID].cycles += __rdtsc() - start;
    g_counters[ID].calls++;
}

#define BEGIN_TIMED_BLOCK(ID) uint64_t startCount##ID = __rdtsc();
#define END_TIMED_BLOCK(ID) EndTimedBlock(MEASURE_##ID, #ID, startCount##ID)
#define FLUSH_COUNTERS() FlushCounters();

#else

#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#define FLUSH_COUNTERS()

#endif
