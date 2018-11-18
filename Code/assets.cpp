//
// Jason Bricco
//

template <typename T>
static inline T* GetAsset(GameState* state, AssetID id)
{
    static_assert(is_base_of<Asset, T>::value);
    return (T*)state->assets[id];
}

static inline void SetAsset(GameState* state, AssetID id, Asset* asset)
{
    state->assets[id] = asset;
}

static void LoadAssets(GameState* state)
{
    Shader* diffuseShader = LoadShader("Shaders\\diffuse_array.shader");
    diffuseShader->view = glGetUniformLocation(diffuseShader->handle, "view");
    diffuseShader->model = glGetUniformLocation(diffuseShader->handle, "model");
    diffuseShader->proj = glGetUniformLocation(diffuseShader->handle, "projection");
    SetAsset(state, ASSET_DIFFUSE_SHADER, diffuseShader);

    Shader* fluidShader = LoadShader("Shaders\\fluid_array.shader");
    fluidShader->view = glGetUniformLocation(fluidShader->handle, "view");
    fluidShader->model = glGetUniformLocation(fluidShader->handle, "model");
    fluidShader->proj = glGetUniformLocation(fluidShader->handle, "projection");
    fluidShader->time = glGetUniformLocation(fluidShader->handle, "time");
    SetAsset(state, ASSET_FLUID_SHADER, fluidShader);

    Shader* crosshair = LoadShader("Shaders\\crosshair.shader");
    crosshair->model = glGetUniformLocation(crosshair->handle, "model");
    crosshair->proj = glGetUniformLocation(crosshair->handle, "projection");
    SetAsset(state, ASSET_CROSSHAIR_SHADER, crosshair);

    Shader* fade = LoadShader("Shaders\\fade.shader");
    fade->color = glGetUniformLocation(fade->handle, "inColor");
    SetAsset(state, ASSET_FADE_SHADER, fade);

    const int texCount = 8;
    char* paths[texCount];

    paths[0] = "Assets/Grass.png";
    paths[1] = "Assets/GrassSide.png";
    paths[2] = "Assets/Dirt.png";
    paths[3] = "Assets/Stone.png";
    paths[4] = "Assets/Water.png";
    paths[5] = "Assets/Sand.png";
    paths[6] = "Assets/Crate.png";
    paths[7] = "Assets/StoneBrick.png";

    Texture* diffuseArray = Malloc<Texture>();
    LoadTextureArray(diffuseArray, paths, texCount, true);
    SetAsset(state, ASSET_DIFFUSE_ARRAY, diffuseArray);

    Texture* crosshairTex = Malloc<Texture>();
    LoadTexture(crosshairTex, "Assets/Crosshair.png");
    SetAsset(state, ASSET_CROSSHAIR, crosshairTex);

    AudioEngine* audio = &state->audio;

    SoundAsset* stoneSound = Calloc<SoundAsset>();
    SetAsset(state, ASSET_STONE_SOUND, stoneSound);
    LoadSound(audio, stoneSound, "Assets/Stone.ogg");

    SoundAsset* leavesSound = Calloc<SoundAsset>();
    SetAsset(state, ASSET_LEAVES_SOUND, leavesSound);
    LoadSound(audio, leavesSound, "Assets/Leaves.ogg");

    LoadMusic(audio, "Assets\\LittleTown.ogg");
}
