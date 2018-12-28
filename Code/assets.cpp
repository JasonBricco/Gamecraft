//
// Jason Bricco
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

static Shader GetShader(GameState* state, ShaderID id)
{
    return state->assets.shaders[id];
}

static void LoadAssets(GameState* state)
{
    char* path = PathToExe("Assets/Assets.gca");

    int size;
    char* data = (char*)ReadFileData(path, &size);

    AssetFileHeader* header = (AssetFileHeader*)data;

    ImageData* imageData = (ImageData*)(data + header->images);
    SoundData* soundData = (SoundData*)(data + header->sounds);
    ShaderData* shaderData = (ShaderData*)(data + header->shaders);

    AssetDatabase& db = state->assets;

    TextureArrayData* arrays = new TextureArrayData[header->arrayCount];

    for (uint32_t i = 0; i < header->imageCount; i++)
    {
        ImageData image = *(imageData + i);

        if (image.array != INT_MAX)
            arrays[image.array].push_back(image);
        else db.images[i] = LoadTexture(image.width, image.height, (uint8_t*)(data + image.pixels));
    }

    for (uint32_t i = 0; i < header->arrayCount; i++)
        db.imageArrays[i] = LoadTextureArray(arrays[i], data);

    delete[] arrays;

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

        db.shaders[i / 2] = LoadShader(vert.length, data + vert.data, frag.length, data + frag.data);
    }

    Shader& diffuseArray = db.shaders[SHADER_DIFFUSE_ARRAY];
    diffuseArray.view = glGetUniformLocation(diffuseArray.handle, "view");
    diffuseArray.model = glGetUniformLocation(diffuseArray.handle, "model");
    diffuseArray.proj = glGetUniformLocation(diffuseArray.handle, "projection");
    diffuseArray.ambient = glGetUniformLocation(diffuseArray.handle, "ambient");

    Shader& fluidArray = db.shaders[SHADER_FLUID_ARRAY];
    fluidArray.view = glGetUniformLocation(fluidArray.handle, "view");
    fluidArray.model = glGetUniformLocation(fluidArray.handle, "model");
    fluidArray.proj = glGetUniformLocation(fluidArray.handle, "projection");
    fluidArray.time = glGetUniformLocation(fluidArray.handle, "time");
    fluidArray.ambient = glGetUniformLocation(fluidArray.handle, "ambient");

    Shader& crosshair = db.shaders[SHADER_CROSSHAIR];
    crosshair.model = glGetUniformLocation(crosshair.handle, "model");
    crosshair.proj = glGetUniformLocation(crosshair.handle, "projection");

    Shader& fade = db.shaders[SHADER_FADE];
    fade.color = glGetUniformLocation(fade.handle, "inColor");

    LoadMusic(audio, "Assets\\LittleTown.ogg");
}
