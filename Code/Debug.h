//
// Gamecraft
// 

#if PROFILING
#pragma message("Profiling enabled.")

#define MAX_DEBUG_RECORDS 128
#define MAX_DEBUG_SNAPSHOTS 256
#define MAX_DEBUG_EVENTS 65536

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

enum DebugEventType : uint8_t
{
    DEBUG_EVENT_BEGIN_BLOCK,
    DEBUG_EVENT_END_BLOCK
};

struct DebugEvent
{
    char* func;
    uint16_t line;
    uint64_t cycles;
    uint16_t threadIndex, coreIndex;
    int recordIndex;
    DebugEventType type;
};

static DebugEvent g_debugEventStorage[2][MAX_DEBUG_EVENTS];

static DebugEvent* g_debugEvents = g_debugEventStorage[0];
static atomic<int> g_debugEventIndex;

struct TimedBlock
{
    char* func;
    int id, line;

    TimedBlock(int id, char* func, int line);
    ~TimedBlock();
};

#define _TIMED_BLOCK(ID, func, line) TimedBlock timedBlock##ID(ID, func, line)
#define TIMED_BLOCK _TIMED_BLOCK(__COUNTER__, __FUNCTION__, __LINE__)

#else

#define TIMED_BLOCK

#endif
