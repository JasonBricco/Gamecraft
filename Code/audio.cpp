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

    asset->music.setVolume(0.0f);
    asset->music.setLoop(true);
}

static inline void PlayMusic(MusicAsset* asset)
{
	asset->music.play();
}

static void ChangeVolume(GameState* state, float target, float seconds)
{
	Music& music = state->currentMusic->music;
	state->volumeLerp = { music.getVolume(), target, 0.0f, seconds };
}

static void UpdateAudio(GameState* state, float deltaTime)
{
	Music& music = state->currentMusic->music;
	LerpData& lData = state->volumeLerp;

	if (lData.t < 1.0f)
	{
		lData.t = Min(lData.t + (deltaTime / lData.time), 1.0f);
		music.setVolume(Lerp(lData.start, lData.end, lData.t));
	}
}
