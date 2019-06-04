//
// Gamecraft
// 

#if PROFILING

static inline void RecordDebugEvent(DebugEventType type, int id, char* func, int line)
{
	DebugTable& t = g_debugTable;
	assert(t.eventIndex + 1 < MAX_DEBUG_EVENTS);
    DebugEvent* event = t.events + t.eventIndex++;
    event->cycles = __rdtsc();
    event->threadID = (uint16_t)GetCurrentThreadId();
    event->recordID = (uint16_t)id;
    event->type = type;

    DebugRecord& record = t.records[id];
    record.func = func;
    record.line = line;
}

static inline void RecordFrameMarker()
{
	DebugTable& t = g_debugTable;
	assert(t.eventIndex + 1 < MAX_DEBUG_EVENTS);
    DebugEvent* event = t.events + t.eventIndex++;
    event->cycles = __rdtsc();
    event->type = DEBUG_EVENT_FRAME_MARKER;
}

TimedFunction::TimedFunction(int id, char* func, int line)
{
	info = { id, func, line };
    RecordDebugEvent(DEBUG_EVENT_BEGIN_BLOCK, id, func, line);
}

TimedFunction::~TimedFunction()
{
	RecordDebugEvent(DEBUG_EVENT_END_BLOCK, info.id, info.func, info.line);
}

static inline DebugThread* GetDebugThread(int threadID)
{
	DebugTable& t = g_debugTable;

	for (int i = 0; i < t.threads.size(); i++)
	{
		DebugThread* thread = t.threads[i];

		if (thread->ID == threadID)
			return thread;
	}

	DebugThread* thread = new DebugThread();
	thread->ID = threadID;
	thread->laneIndex = t.chartLaneCount++;
	t.threads.push_back(thread);

	return thread;
}

static inline DebugRecord* GetRecordFrom(OpeningEvent opening)
{
	return &g_debugTable.records[opening.event.recordID];
}

static void CollateDebugRecords(int inUseIndex)
{
	DebugTable& t = g_debugTable;

	t.frames.clear();
	t.chartScale = 0.0f;
	t.chartLaneCount = 0;

	DebugFrame* currentFrame = nullptr;
	int index = inUseIndex;

	while (true)
	{
		index = (index + 1) % MAX_DEBUG_EVENT_ARRAYS;

		if (index == inUseIndex)
			break;

		int eventCount = t.eventCounts[index];

		for (int e = 0; e < eventCount; e++)
		{
			DebugEvent& event = t.eventStorage[index][e];

			if (event.type == DEBUG_EVENT_FRAME_MARKER)
			{
				if (currentFrame != nullptr)
				{
					currentFrame->endCycles = event.cycles;
					float range = (float)(currentFrame->endCycles - currentFrame->beginCycles);

					if (range > 0.0f)
					{
						float scale = 1.0f / range;

						if (t.chartScale < scale)
							t.chartScale = scale;
					}
				}

				t.frames.emplace_back();
				currentFrame = &t.frames.back();
				currentFrame->beginCycles = event.cycles;
			}
			else if (currentFrame != nullptr)
			{
				int frameIndex = (int)t.frames.size() - 1;

				DebugThread* thread = GetDebugThread(event.threadID);

				switch (event.type)
				{
					case DEBUG_EVENT_BEGIN_BLOCK:
					{
						OpeningEvent open = { event, frameIndex };

						if (thread->openEvents.size() > 0)
							open.parent = GetRecordFrom(thread->openEvents.top());

						thread->openEvents.push(open);
					} break;

					case DEBUG_EVENT_END_BLOCK:
					{
						if (thread->openEvents.size() > 0)
						{
							OpeningEvent matching = thread->openEvents.top();

							if (matching.event.threadID == event.threadID && matching.event.recordID == event.recordID)
							{
								if (matching.frameIndex == frameIndex)
								{
									if (matching.parent == t.scopeToRecord)
									{
										float minT = (float)(matching.event.cycles - currentFrame->beginCycles);
										float maxT = (float)(event.cycles - currentFrame->beginCycles);

										float elapsed = maxT - minT;

										if (elapsed > 0.01f)
										{
											FrameRegion region = { minT, maxT, elapsed, thread->laneIndex, t.records[event.recordID] };
											currentFrame->regions.push_back(region);
										}
									}
								}
								else
								{
									// TODO
								}

								thread->openEvents.pop();
							}
							else
							{
								// TODO							}
							}
						}
					} break;
				}
			}
		}
	}
}

static void DebugEndFrame(GameState* state)
{	
	DebugTable& t = g_debugTable;

	t.eventCounts[t.eventArrayIndex] = t.eventIndex;

	// We will begin writing to a new event array here, and then we'll=
	// process the arrays we're not currently writing to.
	t.eventArrayIndex = (t.eventArrayIndex + 1) % MAX_DEBUG_EVENT_ARRAYS;
	t.events = t.eventStorage[t.eventArrayIndex];

	t.eventIndex = 0;

	if (state->debugDisplay == DEBUG_DISPLAY_PROFILER && t.profilerState >= PROFILER_RECORDING)
	{
		CollateDebugRecords(t.eventArrayIndex);

		if (t.profilerState == PROFILER_RECORD_ONCE)
			t.profilerState = PROFILER_STOPPED;
	}
}

#else

static inline void DebugEndFrame() {}

#endif
