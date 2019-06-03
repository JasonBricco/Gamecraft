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

static char* ProcessCommand(CommandProcessor& processor)
{
	string str(processor.inputText);

	ToLower(str);
	str = Trim(str);

	vector<char*> parts = Split(str, ' ');

	uint64_t hash = FNVHash(parts[0]);
	auto it = processor.commands.find(hash);

	if (it == processor.commands.end())
		return "Unknown command entered.";

	Command cmd = it->second;
	char* msg = cmd.func(cmd.data, parts);

	memset(processor.inputText, 0, sizeof(processor.inputText));

	return msg;
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
	value = strtol(arg, nullptr, 10);

	if (value == 0 || value == LONG_MAX || value == LONG_MIN)
		return false;

	return true;
}

static char* PlayerSpeedCommand(void* data, vector<char*>& args)
{
	if (args.size() != 2) return "Usage: speed <value>";

	float speed;

	if (!IsFloat(args[1], speed))
		return "Invalid argument given to the speed command.";

	Player* player = (Player*)data;
	player->walkSpeed = Clamp(speed, 5.0f, 1000.0f);

	return nullptr;
}

static char* PlayerFlySpeedCommand(void* data, vector<char*>& args)
{
	if (args.size() != 2) return "Usage: flyspeed <value>";

	float speed;

	if (!IsFloat(args[1], speed))
		return "Invalid argument given to the flyspeed command.";

	Player* player = (Player*)data;
	player->flySpeed = Clamp(speed, 5.0f, 1000.0f);

	return nullptr;
}

static char* PlayerHealthCommand(void* data, vector<char*>& args)
{
	if (args.size() != 2) return "Usage: health <value>";

	int health;

	if (!IsInt(args[1], health))
		return "Invalid argument given to the health command.";

	Player* player = (Player*)data;
	player->health = Clamp(health, 1, player->maxHealth);

	return nullptr;
}

static char* PlayerKillCommand(void* data, vector<char*>&)
{
	Player* player = (Player*)data;
	player->health = 0;
	return nullptr;
}

static char* PlayerJumpCommand(void* data, vector<char*>& args)
{
	if (args.size() != 2) return "Usage: jump <value>";

	float vel;

	if (!IsFloat(args[1], vel))
		return "Invalid argument given to the jump command.";

	Player* player = (Player*)data;
	player->jumpVelocity = Clamp(vel, 3.0f, 80.0f);

	return nullptr;
}
