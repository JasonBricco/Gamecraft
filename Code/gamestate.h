//
// Jason Bricco
//

struct GameState
{
	AssetDatabase assets;

	// Holds work to be added by the main thread and performed by background threads.
	WorkQueue workQueue;
	HANDLE semaphore;

	int windowWidth, windowHeight;

	Camera* camera;
	Input input;

	AudioEngine audio;

	float ambient;
	vec3 clearColor;
};
