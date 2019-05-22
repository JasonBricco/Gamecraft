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

struct TimedBlock
{
    uint64_t start;
    char* label;
    bool noFlush;
    int id;

    TimedBlock(int id, char* label, bool noFlush)
    {
        start = __rdtsc();
        this->id = id;
        this->label = label;
        this->noFlush = noFlush;
    }

    ~TimedBlock()
    {
        g_counters[id].label = label;
        g_counters[id].cycles += __rdtsc() - start;
        g_counters[id].calls++;
        g_counters[id].noFlush = noFlush;
    }
};

#define TIMED_BLOCK(ID) TimedBlock timedBlock##ID = TimedBlock(MEASURE_##ID, #ID, false)
#define TIMED_BLOCK_NO_FLUSH(ID) TimedBlock(MEASURE_##ID, #ID, true)
#define FLUSH_COUNTERS() FlushCounters();

#else

#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#define FLUSH_COUNTERS()

#endif
