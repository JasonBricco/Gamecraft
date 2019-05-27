//
// Gamecraft
// 

#if PROFILING

static inline void RecordDebugEvent(DebugEventType type, int id, char* func, int line)
{
	assert(g_debugEventIndex + 1 < MAX_DEBUG_EVENTS);
    DebugEvent* event = g_debugEvents + g_debugEventIndex++;
    event->cycles = __rdtsc();
    event->threadIndex = 0;
    event->coreIndex = 0;
    event->recordIndex = (uint16_t)id;
    event->func = func;
    event->line = (uint16_t)line;
    event->type = type;
}

TimedBlock::TimedBlock(int id, char* func, int line)
{
	this->id = id;
	this->func = func;
	this->line = line;

    RecordDebugEvent(DEBUG_EVENT_BEGIN_BLOCK, id, func, line);
}

TimedBlock::~TimedBlock()
{
    RecordDebugEvent(DEBUG_EVENT_END_BLOCK, id, func, line);
}

static DebugStat BeginDebugStat()
{
	return { 0, LLONG_MAX, LLONG_MIN, 0 };
}

static void EndDebugStat(DebugStat& stat)
{
	if (stat.count == 0)
		stat.min = stat.max = 0;
	else stat.avg /= stat.count;
}

static void UpdateStatistic(DebugStat& stat, int64_t value)
{
	if (value < stat.min)
        stat.min = value;

    if (value > stat.max)
        stat.max = value;

    stat.avg += value;
    stat.count++;
}

static void UpdateDebugStats()
{
	DebugState& state = g_debugState;

	for (int i = 0; i < MAX_DEBUG_RECORDS; i++)
    {
        DebugCounters& c = state.counters[i];

        DebugStat& calls = state.calls[i];
        DebugStat& cycles = state.cycles[i];

        calls = BeginDebugStat();
        cycles = BeginDebugStat();

        for (int j = 0; j < MAX_DEBUG_SNAPSHOTS; j++)
        {
            DebugSnapshot& s = c.snapshots[j];

            if (s.calls > 0)
            {
                UpdateStatistic(calls, s.calls);
                UpdateStatistic(cycles, s.cycles);
            }
        }

        EndDebugStat(calls);
        EndDebugStat(cycles);
    }
}

static void UpdateDebugRecords(DebugEvent* events, int count)
{
	int snapIndex = g_debugState.snapshotIndex;

	for (int i = 0; i < MAX_DEBUG_RECORDS; i++)
	{
		DebugCounters& dest = g_debugState.counters[i];
		dest.snapshots[snapIndex].calls = 0;
		dest.snapshots[snapIndex].cycles = 0;
	}

	for (int i = 0; i < count; i++)
	{
		DebugEvent& event = events[i];
		assert(event.recordIndex >= 0 && event.recordIndex < MAX_DEBUG_RECORDS);

		DebugCounters& dest = g_debugState.counters[event.recordIndex];
		dest.func = event.func;
		dest.line = event.line;

		if (event.type == DEBUG_EVENT_BEGIN_BLOCK)
			dest.snapshots[snapIndex].cycles -= event.cycles;
		else 
		{
			dest.snapshots[snapIndex].calls++;
			dest.snapshots[snapIndex].cycles += event.cycles;
		}
	}
}

static void DebugEndFrame()
{	
	int eventCount = g_debugEventIndex;
	g_debugEventIndex = 0;

	// Swap the event buffer being written into by current timers.
	if (g_debugEvents == g_debugEventStorage[0])
	{
		g_debugEvents = g_debugEventStorage[1];
		UpdateDebugRecords(g_debugEventStorage[0], eventCount);
	}
	else 
	{
		g_debugEvents = g_debugEventStorage[0];
		UpdateDebugRecords(g_debugEventStorage[1], eventCount);
	}

	DebugState& state = g_debugState;

	int& index = state.snapshotIndex;
	index = (index + 1) % MAX_DEBUG_SNAPSHOTS;

	state.framesUntilUpdate--;
}

#else

static inline void DebugEndFrame() {}

#endif
