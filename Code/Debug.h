//
// Gamecraft
// 

#if PROFILING
#pragma message("Profiling enabled.")

#define MAX_CYCLE_COUNTS 128

struct CycleCounter
{
    char* func;
    int line;
    uint64_t cycles;
    uint64_t calls;
};

static CycleCounter g_counters[MAX_CYCLE_COUNTS];

struct TimedBlock
{
    uint64_t start;
    char* func;
    int id, line;

    TimedBlock(int id, char* func, int line)
    {
        start = __rdtsc();
        this->id = id;
        this->func = func;
        this->line = line;
    }

    ~TimedBlock()
    {
        g_counters[id].func = func;
        g_counters[id].line = line;
        g_counters[id].cycles += __rdtsc() - start;
        g_counters[id].calls++;
    }
};

#define _TIMED_BLOCK(ID, func, line) TimedBlock timedBlock##ID(ID, func, line)
#define TIMED_BLOCK _TIMED_BLOCK(__COUNTER__, __FUNCTION__, __LINE__)

#else

#define TIMED_BLOCK(ID)

#endif
