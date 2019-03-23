//
// Gamecraft
//

static Texture GetTexture(GameState* state, ImageID id)
{
    return state->assets.images[id];
}

static Texture GetTextureArray(GameState* state, ImageArrayID id)
{
    return state->assets.imageArrays[id];
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
    char* buffer = AllocTempArray(MAX_PATH, char);
    char* path = PathToExe("Assets/Assets.gca", buffer, MAX_PATH);

    uint32_t size;
    void* dataPtr = ReadFileData(path, &size);

    if (dataPtr == nullptr)
        Error("Asset file not found! Ensure the file Assets.gca is in the Assets folder in the same directory as the executable.\n");

    char* data = (char*)dataPtr;
    AssetFileHeader* header = (AssetFileHeader*)data;

    if (header->code != (FORMAT_CODE('g', 'c', 'a', 'f')))
        Error("Invalid asset file. The file may be damaged.\n");

    if (header->version > 1)
        Error("Asset file version is unsupported.\n");

    ImageData* imageData = (ImageData*)(data + header->images);
    SoundData* soundData = (SoundData*)(data + header->sounds);
    ShaderData* shaderData = (ShaderData*)(data + header->shaders);

    AssetDatabase& db = state->assets;

    ImageData* array = AllocTempArray(header->arrayCount, ImageData);
    int arrayCount = 0;

    for (uint32_t i = 0; i < header->imageCount; i++)
    {
        ImageData image = *(imageData + i);

        if (image.array != INT_MAX)
            array[arrayCount++] = image;
        
        db.images[i] = LoadTexture(image.width, image.height, (uint8_t*)(data + image.pixels));
    }

    for (uint32_t i = 0; i < header->arrayCount; i++)
        db.imageArrays[i] = LoadTextureArray(array, arrayCount, data);

    AudioEngine* audio = &state->audio;
    
    for (uint32_t i = 0; i < header->soundCount; i++)
    {
        SoundData sound = *(soundData + i);
        db.sounds[i] = { audio, (int16_t*)(data + sound.samples), sound.sampleCount, sound.sampleRate };
    }

    for (uint32_t i = 0; i < header->shaderCount; i += 2)
    {
        ShaderData vert = *(shaderData + i);
        ShaderData frag = *(shaderData + i + 1);

        LoadShader(&db.shaders[i / 2], vert.length, data + vert.data, frag.length, data + frag.data);
        assert(glIsProgram(db.shaders[i / 2].handle));
    }

    LoadMusic(audio, "Assets\\LittleTown.ogg");
}
