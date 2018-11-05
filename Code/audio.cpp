//
// Jason Bricco
//

static void LoadSound(SoundAsset* asset, char* path)
{
	if (!asset->buffer.loadFromFile(PathToExe(path)))
    	Print("Failed to load sound at %s\n", path);

    asset->sound.setBuffer(asset->buffer);
}

static inline void PlaySound(SoundAsset* asset)
{
	asset->sound.play();
}

static void LoadMusic(MusicAsset* asset, char* path)
{
	if (!asset->music.openFromFile(PathToExe(path)))
    	Print("Failed to load music at %s\n", path);

    asset->music.setVolume(75.0f);
    asset->music.setLoop(true);
}

static inline void PlayMusic(MusicAsset* asset)
{
	asset->music.play();
}

static void ChangeVolume(GameState* state, float target, float seconds)
{
	// TODO: Change volume of state->currentMusic by fading it from its current volume 
	// to the target volume over 'seconds' seconds.
}
