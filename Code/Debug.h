//
// Gamecraft
// 

#if PROFILING
#pragma message("Profiling enabled.")

#define MAX_DEBUG_RECORDS 128
#define MAX_DEBUG_SNAPSHOTS 64

struct DebugStat
{
    int count;
    int64_t min, max, avg;
};

struct DebugSnapshot
{
    atomic<uint64_t> cycles;
    atomic<uint64_t> calls;
};

struct DebugCounters
{
    char* func;
    int line;

    DebugSnapshot snapshots[MAX_DEBUG_SNAPSHOTS];
};

struct DebugState
{
    int framesUntilUpdate, snapshotIndex;
    DebugCounters counters[MAX_DEBUG_RECORDS];

    DebugStat calls[MAX_DEBUG_RECORDS];
    DebugStat cycles[MAX_DEBUG_RECORDS];
};

static DebugState g_debugState;

struct DebugRecord
{
    char* func;
    int line;
    atomic<uint64_t> cycles;
    atomic<uint64_t> calls;
};

// Debug records for the given frame.
static DebugRecord g_debugRecords[MAX_DEBUG_RECORDS];

struct TimedBlock
{
    uint64_t start;
    char* func;
    int id, line;

    TimedBlock(int id, char* func, int line)
    {
        assert(id < MAX_DEBUG_RECORDS);
        start = __rdtsc();
        this->id = id;
        this->func = func;
        this->line = line;
    }

    ~TimedBlock()
    {
        DebugRecord& record = g_debugRecords[id];
        record.func = func;
        record.line = line;
        record.cycles += __rdtsc() - start;
        record.calls++;
    }
};

#define _TIMED_BLOCK(ID, func, line) TimedBlock timedBlock##ID(ID, func, line)
#define TIMED_BLOCK _TIMED_BLOCK(__COUNTER__, __FUNCTION__, __LINE__)

#else

#define TIMED_BLOCK

#endif
