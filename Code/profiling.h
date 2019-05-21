//
// Gamecraft
// 

#if PROFILING
#pragma message("Profiling enabled.")

enum MeasureSection
{
    MEASURE_GAME_LOOP,
    MEASURE_PLAYER_MOVE,
    MEASURE_DRAW_PARTICLES,
    MEASURE_RENDER_SCENE,
    MEASURE_COUNT
};

struct CycleCounter
{
    char* label;
    uint64_t cycles;
    uint64_t calls;
    bool noFlush;
};

static CycleCounter g_counters[MEASURE_COUNT];

inline void EndTimedBlock(int ID, char* label, uint64_t start, bool noFlush = false)
{
    g_counters[ID].label = label;
    g_counters[ID].cycles += __rdtsc() - start;
    g_counters[ID].calls++;
    g_counters[ID].noFlush = noFlush;
}

#define BEGIN_TIMED_BLOCK(ID) uint64_t startCount##ID = __rdtsc();
#define END_TIMED_BLOCK(ID) EndTimedBlock(MEASURE_##ID, #ID, startCount##ID)
#define END_TIMED_BLOCK_NO_FLUSH(ID) EndTimedBlock(MEASURE_##ID, #ID, startCount##ID, true)
#define FLUSH_COUNTERS() FlushCounters();

#else

#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#define FLUSH_COUNTERS()

#endif
