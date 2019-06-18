//
// Gamecraft
//

// Fowler–Noll–Vo hash.
static uint64_t FNVHash(char* s)
{
	uint64_t hash = 14695981039346656037;

	for (char* c = s; *c != '\0'; c++)
	{
		hash *= 1099511628211;
		hash ^= *c;
	}

	return hash;
}

static void RegisterCommand(GameState* state, char* text, CommandFunc func, void* data)
{
	CommandProcessor& processor = state->cmdProcessor;
	Command cmd = { func, data };
	processor.commands.insert(make_pair(FNVHash(text), cmd));
}

static CommandResult ProcessCommand(GameState* state, CommandProcessor& processor)
{
	string str(processor.inputText);

	ToLower(str);
	str = Trim(str);

	if (str.size() == 0)
		return { nullptr, PLAYING };

	vector<char*> parts = Split(str, ' ');

	uint64_t hash = FNVHash(parts[0]);
	auto it = processor.commands.find(hash);

	if (it == processor.commands.end())
		return { "Unknown command entered." };

	Command cmd = it->second;
	CommandResult result = cmd.func(state, cmd.data, parts);

	memset(processor.inputText, 0, sizeof(processor.inputText));

	return result;
}

static inline bool IsFloat(char* arg, float& value)
{
	value = strtof(arg, nullptr);

	if (value == 0.0f || value == HUGE_VALF || value == -HUGE_VALF)
		return false;

	return true;
}

static inline bool IsInt(char* arg, int& value)
{
	char* endPtr;
	value = strtol(arg, &endPtr, 10);

	if ((value == 0 && endPtr == arg) || value == LONG_MAX || value == LONG_MIN)
		return false;

	return true;
}

static inline bool StringEquals(char* a, char* b)
{
	return strcmp(a, b) == 0;
}

static CommandResult HelpCommand(GameState* state, void*, vector<char*>&)
{
	state->cmdProcessor.help = true;
	return { nullptr, ENTERING_COMMAND };
}

static CommandResult PlayerSpeedCommand(GameState*, void* playerPtr, vector<char*>& args)
{
	if (args.size() != 2) return { "Usage: speed <value>" };

	float speed;

	if (!IsFloat(args[1], speed))
		return { "Invalid argument given to the speed command." };

	Player* player = (Player*)playerPtr;
	player->walkSpeed = Clamp(speed, 5.0f, 1000.0f);

	return { nullptr };
}

static CommandResult PlayerFlySpeedCommand(GameState*, void* playerPtr, vector<char*>& args)
{
	if (args.size() != 2) return { "Usage: flyspeed <value>" };

	float speed;

	if (!IsFloat(args[1], speed))
		return { "Invalid argument given to the flyspeed command." };

	Player* player = (Player*)playerPtr;
	player->flySpeed = Clamp(speed, 5.0f, 1000.0f);

	return { nullptr };
}

static CommandResult PlayerHealthCommand(GameState*, void* playerPtr, vector<char*>& args)
{
	if (args.size() != 2) return { "Usage: health <value>" };

	int health;

	if (!IsInt(args[1], health))
		return { "Invalid argument given to the health command." };

	Player* player = (Player*)playerPtr;
	player->health = Clamp(health, 0, player->maxHealth);

	return { nullptr };
}

static CommandResult PlayerKillCommand(GameState*, void* playerPtr, vector<char*>&)
{
	Player* player = (Player*)playerPtr;
	player->health = 0;
	return { nullptr };
}

static CommandResult PlayerJumpCommand(GameState*, void* playerPtr, vector<char*>& args)
{
	if (args.size() != 2) return { "Usage: jump <value>" };

	float vel;

	if (!IsFloat(args[1], vel))
		return { "Invalid argument given to the jump command." };

	Player* player = (Player*)playerPtr;
	player->jumpVelocity = Clamp(vel, 3.0f, 80.0f);

	return { nullptr };
}

static CommandResult PlayerTeleportCommand(GameState* state, void* worldPtr, vector<char*>& args)
{
	if (args.size() == 2)
	{
		if (StringEquals(args[1], "home"))
		{
			World* world = (World*)worldPtr;
			TeleportHome(state, world);

			return { nullptr, LOADING };
		}
		else return { "Invalid argument given to the teleport command." };
	}
	else if (args.size() == 4)
	{
		WorldP worldP;

		for (int i = 1; i < 4; i++)
		{
			if (!IsInt(args[i], worldP[i - 1]))
				return { "Invalid argument given to the teleport command." };
		}

		worldP.y = Clamp(worldP.y, 1, 512);

		WorldLocation loc = { worldP, WorldToRelP(worldP) };
		TeleportPlayer(state, loc);

		return { nullptr, LOADING };
	}
	else return { "Usage: teleport <x> <y> <z> or teleport home" };
}

static CommandResult SetHomeCommand(GameState*, void* worldPtr, vector<char*>&)
{
	World* world = (World*)worldPtr;
	SetHomePos(world, world->player);
	return { nullptr };
}

#if DEBUG_SERVICES

static CommandResult ChunkOutlinesCommand(GameState*, void*, vector<char*>&)
{
	 g_debugTable.showOutlines = !g_debugTable.showOutlines;
	 return { nullptr };
}

static void StopProfilerCommand(GameState* state, void* windowPtr)
{
	GLFWwindow* window = (GLFWwindow*)windowPtr;
	state->savedInputMode = glfwGetInputMode(window, GLFW_CURSOR);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    g_debugTable.profilerState = PROFILER_STOPPED;
}

static void StartProfilerCommand(GameState* state, void* windowPtr)
{
	GLFWwindow* window = (GLFWwindow*)windowPtr;
	glfwSetInputMode(window, GLFW_CURSOR, state->savedInputMode);
    CenterCursor();
    g_debugTable.profilerState = PROFILER_RECORDING;
    state->debugDisplay = true;
}

static CommandResult ProfilerCommand(GameState* state, void* windowPtr, vector<char*>& args)
{
	if (args.size() != 2)
		return { "Usage: profiler <start, stop, hide>" };

	DebugTable& t = g_debugTable;

	if (StringEquals(args[1], "hide"))
	{
		t.profilerState = PROFILER_HIDDEN;
		return { nullptr };
	}

	if (StringEquals(args[1], "stop"))
	{
		if (t.profilerState != PROFILER_HIDDEN)
			StopProfilerCommand(state, windowPtr);
		
        return { nullptr, PLAYING };
	}

	if (StringEquals(args[1], "start"))
	{
		StartProfilerCommand(state, windowPtr);
        return { nullptr };
	}

	return { "Invalid argument given to the profiler command." };
}

static CommandResult FastProfilerToggleCommand(GameState* state, void* windowPtr, vector<char*>&)
{
	DebugTable& t = g_debugTable;

	if (t.profilerState == PROFILER_STOPPED)
		StartProfilerCommand(state, windowPtr);
	else if (t.profilerState == PROFILER_RECORDING)
		StopProfilerCommand(state, windowPtr);

	return { nullptr };
}

#endif
