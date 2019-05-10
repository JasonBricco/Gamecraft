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

	// Holds work to be added by the main thread and performed by background threads,
	// or work added by background threads to be performed by the main thread.
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

	float deltaTime;

	ParticleEmitter rain;
	ParticleEmitter snow;

	PauseState pauseState;
	LoadingInfo loadInfo;

	char* savePath;

	bool debugHudActive;

	WorldLocation teleportLoc;
	WorldConfig* pendingConfig;
};
