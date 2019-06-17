//
// Gamecraft
// 

#if DEBUG_SERVICES

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

static void CreateDebugMesh()
{
	vector<u8vec3> data;

	data.push_back(u8vec3(0, 0, 1));
    data.push_back(u8vec3(0, 1, 1));

    data.push_back(u8vec3(0, 1, 1));
    data.push_back(u8vec3(1, 1, 1));

    data.push_back(u8vec3(1, 1, 1));
    data.push_back(u8vec3(1, 0, 1));

    data.push_back(u8vec3(1, 0, 1));
    data.push_back(u8vec3(0, 0, 1));

    data.push_back(u8vec3(0, 0, 1));
    data.push_back(u8vec3(0, 0, 0));

    data.push_back(u8vec3(0, 0, 0));
    data.push_back(u8vec3(1, 0, 0));

    data.push_back(u8vec3(1, 0, 0));
    data.push_back(u8vec3(1, 0, 1));

    data.push_back(u8vec3(0, 0, 0));
    data.push_back(u8vec3(0, 1, 0));

    data.push_back(u8vec3(0, 1, 0));
    data.push_back(u8vec3(1, 1, 0));

    data.push_back(u8vec3(1, 1, 0));
    data.push_back(u8vec3(1, 0, 0));

    data.push_back(u8vec3(1, 1, 0));
    data.push_back(u8vec3(1, 1, 1));

    data.push_back(u8vec3(0, 1, 0));
    data.push_back(u8vec3(0, 1, 1));

    DebugMesh& mesh = g_debugTable.outlineMesh;

    glGenVertexArrays(1, &mesh.va);
    glBindVertexArray(mesh.va);

    glGenBuffers(1, &mesh.vertices);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(u8vec3) * data.size(), data.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	mesh.count = (int)data.size();
}

static void CreateDebugShader()
{
	char* vert = 
		"#version 440 core\n"
		"layout (location = 0) in vec3 pos;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{\n"
			"gl_Position = projection * view * model * vec4(pos, 1.0);\n"
		"}\n";

	char* frag =
		"#version 440 core\n"
		"out vec4 outColor;\n"
		"uniform vec4 color;\n"
		"void main()\n"
		"{\n"
			"outColor = color;\n"
		"}\n";

	int vertLength = (int)strlen(vert);
	int fragLength = (int)strlen(frag);

	GLuint vS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vS, 1, &vert, &vertLength);
    glCompileShader(vS);

    if (ShaderHasErrors(vS, VERTEX_SHADER))
       	Error("Failed to compile the debug vertex shader.\n");

    GLuint fS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fS, 1, &frag, &fragLength);
    glCompileShader(fS);
    
    if (ShaderHasErrors(fS, FRAGMENT_SHADER))
        Error("Failed to compile the debug fragment shader.\n");

    GLuint program = glCreateProgram();
    glAttachShader(program, vS);
    glAttachShader(program, fS);
    glLinkProgram(program);
    
    if (ShaderHasErrors(program, SHADER_PROGRAM))
        Error("Failed to link the debug shaders into the program.\n");

    glDeleteShader(vS);
    glDeleteShader(fS);

    DebugShader& shader = g_debugTable.shader;
    shader.handle = program;
    shader.view = glGetUniformLocation(shader.handle, "view");
    shader.model = glGetUniformLocation(shader.handle, "model");
    shader.proj = glGetUniformLocation(shader.handle, "projection");
    shader.color = glGetUniformLocation(shader.handle, "color");
}

static void DrawDebugMesh(DebugMesh& mesh, vec3 pos, vec3 size, Color color)
{
	DebugShader& shader = g_debugTable.shader;

	mat4 model = translate(mat4(1.0f), pos);
	model = scale(model, size);

	SetUniform(shader.model, model);
	SetUniform(shader.color, color);

	glBindVertexArray(mesh.va);
	glDrawArrays(GL_LINES, 0, mesh.count);
}

static void DrawChunkOutline(Chunk* chunk)
{
	if (g_debugTable.showOutlines)
	{
		DebugOutline outline = { chunk->lwPos, vec3(CHUNK_SIZE_H, CHUNK_SIZE_V, CHUNK_SIZE_H), RED_COLOR };
		g_debugTable.outlines.push_back(outline);
	}
}

static void DebugInit(GameState* state, GLFWwindow* window)
{
	CreateDebugMesh();
	CreateDebugShader();

	RegisterCommand(state, "outlines", ChunkOutlinesCommand, nullptr);
	RegisterCommand(state, "profiler", ProfilerCommand, window);
	RegisterCommand(state, "p", FastProfilerToggleCommand, window);
}

static void DebugDraw(Renderer& rend, Camera* cam)
{
	DebugShader& shader = g_debugTable.shader;

	glUseProgram(shader.handle);
	SetUniform(shader.proj, rend.perspective);
	SetUniform(shader.view, cam->view);

	DebugMesh& mesh = g_debugTable.outlineMesh;
	auto& outlines = g_debugTable.outlines;

	for (DebugOutline outline : outlines)
		DrawDebugMesh(mesh, outline.pos, outline.scale, outline.color);
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

	if (state->debugDisplay && t.profilerState >= PROFILER_RECORDING)
	{
		CollateDebugRecords(t.eventArrayIndex);

		if (t.profilerState == PROFILER_RECORD_ONCE)
			t.profilerState = PROFILER_STOPPED;
	}

	g_debugTable.outlines.clear();
}

#endif
