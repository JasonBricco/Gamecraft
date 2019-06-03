//
// Gamecraft
//

#define MAX_COMMAND_LENGTH 100

using CommandFunc = char*(*)(void* data, vector<char*>& args);

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
static char* ProcessCommand(CommandProcessor& processor);

// Command functions.
static char* PlayerSpeedCommand(void* data, vector<char*>& args);
static char* PlayerHealthCommand(void* data, vector<char*>& args);
static char* PlayerFlySpeedCommand(void* data, vector<char*>& args);
static char* PlayerKillCommand(void* data, vector<char*>& args);
static char* PlayerJumpCommand(void* data, vector<char*>&);
