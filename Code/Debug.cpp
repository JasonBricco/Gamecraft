//
// Gamecraft
// 

#if PROFILING

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

static void UpdateDebugRecords()
{
	int snapIndex = g_debugState.snapshotIndex;

	for (int i = 0; i < MAX_DEBUG_RECORDS; i++)
	{
		DebugRecord& record = g_debugRecords[i];
		DebugCounters& dest = g_debugState.counters[i];

		dest.func = record.func;
		dest.line = record.line;
		dest.snapshots[snapIndex].calls.store(record.calls);
		dest.snapshots[snapIndex].cycles.store(record.cycles);

		record.calls = 0;
		record.cycles = 0;
	}
}

static void DebugEndFrame()
{
	UpdateDebugRecords();

	DebugState& state = g_debugState;

	int& index = state.snapshotIndex;
	index = (index + 1) % MAX_DEBUG_SNAPSHOTS;

	state.framesUntilUpdate--;
}

#else

static inline void DebugEndFrame() {}

#endif
