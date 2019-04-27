//
// Gamecraft
//

enum PauseState
{
	PLAYING,
	PAUSED,
	SELECTING_BLOCK,
	GAME_SETTINGS,
	WORLD_CONFIG
};

struct GameState
{
	AssetDatabase assets;

	// Holds work to be added by the main thread and performed by background threads,
	// or work added by background threads to be performed by the main thread.
	AsyncWorkQueue workQueue;
	
	List<AsyncCallbackItem> callbacks;
	SRWLOCK callbackLock;
	
	HANDLE semaphore;

	int windowWidth, windowHeight;

	Renderer renderer;

	Camera* camera;
	Input input;

	AudioEngine audio;
	UI ui;

	float ambient;
	vec3 clearColor;

	float deltaTime;

	ParticleEmitter rain;

	PauseState pauseState;

	char* savePath;

	bool debugHudActive;
};
