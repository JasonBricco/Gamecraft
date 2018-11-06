//
// Jason Bricco
//

struct GameState
{
	Asset* assets[ASSET_COUNT];

	// Holds work to be added by the main thread and performed by background threads.
	WorkQueue workQueue;
	HANDLE semaphore;

	int windowWidth, windowHeight;

	Camera* camera;
	Input input;

	MusicAsset* currentMusic;
	LerpData volumeLerp;
};
