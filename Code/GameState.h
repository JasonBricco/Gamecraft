//
// Gamecraft
//

enum PauseState
{
	PLAYING,
	PAUSED,
	SELECTING_BLOCK,
	GAME_SETTINGS,
	WORLD_CONFIG,
	ENTERING_COMMAND,
	PLAYER_DEAD,
	LOADING
};

enum LoadingState
{
	LOADING_NONE,
	LOADING_FADE_IN,
	LOADING_FADE_OUT
};

struct LoadingInfo
{
	LoadingState state;
	Color initialFade;
	float t, fadeTime;
	function<void(GameState*, World*)> callback;
};

struct GameState
{
	AssetDatabase assets;
	AsyncWorkQueue workQueue;
	
	vector<AsyncCallbackItem> callbacks;
	SRWLOCK callbackLock;
	
	HANDLE semaphore;

	bool minimized;

	Renderer renderer;

	Camera* camera;
	Input input;

	AudioEngine audio;
	UI ui;

	CommandProcessor cmdProcessor;

	float time, deltaTime;

	ParticleEmitter rain;
	ParticleEmitter snow;

	PauseState pauseState;
	LoadingInfo loadInfo;

	char* savePath;

	bool debugDisplay;

	WorldLocation teleportLoc;
	WorldConfig* pendingConfig;

	int savedInputMode;
};
