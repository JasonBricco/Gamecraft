//
// Gamecraft
//

static Texture GetTexture(GameState* state, ImageID id)
{
    return state->assets.images[id];
}

static Texture GetBlockTextureArray(GameState* state)
{
    return state->assets.blockArray;
}

static Sound GetSound(GameState* state, SoundID id)
{
    return state->assets.sounds[id];
}

static Shader* GetShader(GameState* state, ShaderID id)
{
    return &state->assets.shaders[id];
}

static void LoadAssets(GameState* state)
{
    char buffer[MAX_PATH];
    char* path = PathToExe("Assets/Assets.gca", buffer, MAX_PATH);

    uint32_t size;
    void* dataPtr = ReadFileData(path, &size);

    if (dataPtr == nullptr)
        Error("Asset file not found! Ensure the file Assets.gca is in the Assets folder in the same directory as the executable.\n");

    char* data = (char*)dataPtr;
    AssetFileHeader* header = (AssetFileHeader*)data;

    if (header->code != (FORMAT_CODE('g', 'c', 'a')))
        Error("Invalid asset file. The file may be damaged.\n");

    if (header->version > 1)
        Error("Asset file version is unsupported.\n");

    AssetDatabase& db = state->assets;

    // Load block images.
    ImageData* blockData = (ImageData*)(data + header->blockImages);
    
    int count = 0;

    for (uint32_t i = 0; i < header->blockImageCount; i++)
    {
        ImageData image = *(blockData + i);
        db.images[count++] = LoadTexture(image.width, image.height, (uint8_t*)(data + image.pixels));

        if (image.customMips)
        {
            i += 5;
            continue;
        }
    }
       
    db.blockArray = LoadTextureArray(blockData, count, header->blockImageCount, (uint8_t*)data);

    // Load images.
    ImageData* imageData = (ImageData*)(data + header->images);

    for (uint32_t i = 0; i < header->imageCount; i++)
    {
        ImageData image = *(imageData + i);
        db.images[count + i] = LoadTexture(image.width, image.height, (uint8_t*)(data + image.pixels));
    }

    // Load sounds.
    AudioEngine* audio = &state->audio;
    SoundData* soundData = (SoundData*)(data + header->sounds);

    for (uint32_t i = 0; i < header->soundCount; i++)
    {
        SoundData sound = *(soundData + i);
        db.sounds[i] = { audio, (int16_t*)(data + sound.samples), sound.sampleCount, sound.sampleRate };
    }

    // Load shaders.
    ShaderData* shaderData = (ShaderData*)(data + header->shaders);

    for (uint32_t i = 0; i < header->shaderCount; i += 2)
    {
        ShaderData vert = *(shaderData + i);
        ShaderData frag = *(shaderData + i + 1);

        LoadShader(&db.shaders[i / 2], vert.length, data + vert.data, frag.length, data + frag.data);
        assert(glIsProgram(db.shaders[i / 2].handle));
    }

    LoadMusic(audio, "Assets\\LittleTown.ogg");
}
