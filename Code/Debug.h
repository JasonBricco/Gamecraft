//
// Gamecraft
// 

#if PROFILING
#pragma message("Profiling enabled.")

#define MAX_DEBUG_EVENTS 65536
#define MAX_DEBUG_EVENT_ARRAYS 32

enum DebugEventType : uint8_t
{
    DEBUG_EVENT_BEGIN_BLOCK,
    DEBUG_EVENT_END_BLOCK,
    DEBUG_EVENT_FRAME_MARKER
};

struct DebugEvent
{
    DebugEventType type;
    char* func;
    uint64_t cycles;
    uint16_t line;
    uint16_t threadID;
    uint16_t recordID;
};

// Represents a timespan being recorded during the frame.
struct FrameRegion
{
    float minT, maxT;
    char* func;
    float elapsed;
    int line, laneIndex;
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
};

struct DebugThread
{
    int ID, laneIndex;
    stack<OpeningEvent> openEvents;
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

    int collationIndex;
    bool recording = true;
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

#define START_PROFILING() g_debugTable.recording = true
#define STOP_PROFILING() g_debugTable.recording = false

#else

#define BEGIN_BLOCK(ID)
#define END_BLOCK(ID)
#define FRAME_MARKER
#define TIMED_FUNCTION

#define START_PROFILING()
#define STOP_PROFILING()

#endif
