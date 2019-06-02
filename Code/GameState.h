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
	VIEWING_PROFILE,
	PLAYER_DEAD,
	LOADING
};

enum DebugDisplay
{
	DEBUG_DISPLAY_NONE,
	DEBUG_DISPLAY_HUD,
	DEBUG_DISPLAY_PROFILER
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

	float time, deltaTime;

	ParticleEmitter rain;
	ParticleEmitter snow;

	PauseState pauseState;
	LoadingInfo loadInfo;

	char* savePath;

	DebugDisplay debugDisplay;

	WorldLocation teleportLoc;
	WorldConfig* pendingConfig;
};
