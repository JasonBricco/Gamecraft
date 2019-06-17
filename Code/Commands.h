//
// Gamecraft
//

#define MAX_COMMAND_LENGTH 100

struct CommandResult
{
	char* error;
	int newState;
};
	
using CommandFunc = CommandResult(*)(GameState* state, void* data, vector<char*>& args);

struct Command
{
	CommandFunc func;
	void* data;
};

struct CommandProcessor
{
	char inputText[MAX_COMMAND_LENGTH];
	unordered_map<uint64_t, Command> commands;

	float errorTime;
    char* error;
};

static void RegisterCommand(GameState* state, char* text, CommandFunc func, void* data);
static CommandResult ProcessCommand(GameState* state, CommandProcessor& processor);

// Command functions.
static CommandResult PlayerSpeedCommand(GameState* state, void* playerPtr, vector<char*>& args);
static CommandResult PlayerHealthCommand(GameState* state, void* playerPtr, vector<char*>& args);
static CommandResult PlayerFlySpeedCommand(GameState* state, void* playerPtr, vector<char*>& args);
static CommandResult PlayerKillCommand(GameState* state, void* playerPtr, vector<char*>& args);
static CommandResult PlayerJumpCommand(GameState* state, void* playerPtr, vector<char*>&);
static CommandResult PlayerTeleportCommand(GameState* state, void* worldPtr, vector<char*>&);
static CommandResult SetHomeCommand(GameState* state, void* worldPtr, vector<char*>& args);

#if DEBUG_SERVICES
static CommandResult ChunkOutlinesCommand(GameState*, void*, vector<char*>&);
static CommandResult ProfilerCommand(GameState* state, void* windowPtr, vector<char*>& args);
static CommandResult FastProfilerToggleCommand(GameState* state, void* windowPtr, vector<char*>&);
#endif
