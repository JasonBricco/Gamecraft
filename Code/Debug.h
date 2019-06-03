//
// Gamecraft
// 

#if PROFILING
#pragma message("Profiling enabled.")

#define MAX_DEBUG_EVENTS 65536
#define MAX_DEBUG_EVENT_ARRAYS 32
#define MAX_DEBUG_RECORDS 256

enum DebugEventType : uint8_t
{
    DEBUG_EVENT_BEGIN_BLOCK,
    DEBUG_EVENT_END_BLOCK,
    DEBUG_EVENT_FRAME_MARKER
};

struct DebugRecord
{
    char* func;
    int line;
};

struct DebugEvent
{
    DebugEventType type;
    uint64_t cycles;
    uint16_t threadID;
    uint16_t recordID;
};

// Represents a timespan being recorded during the frame.
struct FrameRegion
{
    float minT, maxT;
    float elapsed;
    int laneIndex;
    DebugRecord& record;
};

struct DebugFrame
{
    uint64_t beginCycles, endCycles;
    vector<FrameRegion> regions;
};

struct OpeningEvent
{
    DebugEvent& event;
    int frameIndex;
    DebugRecord* parent;
};

struct DebugThread
{
    int ID, laneIndex;
    stack<OpeningEvent> openEvents;
};

enum ProfilerState
{
    PROFILER_STOPPED,
    PROFILER_RECORDING,
    PROFILER_RECORD_ONCE
};

struct DebugTable
{
    // As the code runs, it can report debug events. These events store 
    // the current "CPU time", the thread they're on, and a given type.
    // This array is a running log of these events. On each frame we 
    // start writing into a new array, wrapping which array we write to
    // around as in a circular buffer.
    DebugEvent eventStorage[MAX_DEBUG_EVENT_ARRAYS][MAX_DEBUG_EVENTS];

    // Stores the number of events we have in each of the event arrays.
    int eventCounts[MAX_DEBUG_EVENT_ARRAYS];

    // Tracks which array we're currently writing debug events into. 
    // The event index is the location in that array we are writing to.
    DebugEvent* events = eventStorage[0];
    atomic<int> eventIndex;

    // The index of the current event array we're writing into.
    int eventArrayIndex;

    int chartLaneCount;
    float chartScale;
    
    vector<DebugFrame> frames;
    vector<DebugThread*> threads;

    DebugRecord* scopeToRecord;
    DebugRecord records[MAX_DEBUG_RECORDS];

    ProfilerState profilerState = PROFILER_RECORDING;
};

static DebugTable g_debugTable;

struct TimedInfo
{
    int id;
    char* func;
    int line;
};

struct TimedFunction
{
    TimedInfo info;

    TimedFunction(int id, char* func, int line);
    ~TimedFunction();
};

#define BEGIN_BLOCK(ID) \
    TimedInfo info##ID = { __COUNTER__, #ID, __LINE__ }; \
    RecordDebugEvent(DEBUG_EVENT_BEGIN_BLOCK, info##ID.id, info##ID.func, info##ID.line)

#define END_BLOCK(ID) RecordDebugEvent(DEBUG_EVENT_END_BLOCK, info##ID.id, info##ID.func, info##ID.line)

#define FRAME_MARKER RecordFrameMarker()

#define _TIMED_FUNCTION(ID, func, line) TimedFunction timedFunction##ID(ID, func, line)
#define TIMED_FUNCTION _TIMED_FUNCTION(__COUNTER__, __FUNCTION__, __LINE__)

#define START_PROFILING() g_debugTable.profilerState = PROFILER_RECORDING
#define STOP_PROFILING() g_debugTable.profilerState = PROFILER_STOPPED
#define IS_PROFILING() (g_debugTable.profilerState == PROFILER_RECORDING)

#else

#define BEGIN_BLOCK(ID)
#define END_BLOCK(ID)
#define FRAME_MARKER
#define TIMED_FUNCTION

#define START_PROFILING()
#define STOP_PROFILING()
#define IS_PROFILING() false

#endif
