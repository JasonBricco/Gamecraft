//
// Jason Bricco
//

enum AssetID
{
	ASSET_EMPTY,
	ASSET_DIFFUSE_ARRAY,
	ASSET_CROSSHAIR,
	ASSET_DIFFUSE_SHADER,
	ASSET_FLUID_SHADER,
	ASSET_CROSSHAIR_SHADER,
	ASSET_FADE_SHADER,
	ASSET_STONE_SOUND,
	ASSET_LEAVES_SOUND,
	ASSET_MUSIC,
	ASSET_COUNT
};

struct Asset {};

struct Shader : public Asset
{
    GLuint handle;
    GLint view, model, proj, time, color;
};

struct Texture : public Asset
{
	GLuint id;
};

struct SoundAsset : public Asset
{
	SoundBuffer soundBuffer;
	Sound sound;

	void Load(char* path)
	{
		if (!soundBuffer.loadFromFile(PathToExe(path)))
        	Print("Failed to load sound at %s\n", path);

        sound.setBuffer(soundBuffer);
	}

	void Play()
	{
		sound.play();
	}
};

struct MusicAsset : public Asset
{
	Music music;

	void Load(char* path)
	{
		if (!music.openFromFile(PathToExe(path)))
        	Print("Failed to load music at %s\n", path);

        music.setVolume(75.0f);
        music.setLoop(true);
	}

	void Play()
	{
		music.play();
	}
};
