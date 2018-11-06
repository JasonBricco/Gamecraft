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

    char** paths = NULL;

    sb_push(paths, "Assets/Grass.png");
    sb_push(paths, "Assets/GrassSide.png");
    sb_push(paths, "Assets/Dirt.png");
    sb_push(paths, "Assets/Stone.png");
    sb_push(paths, "Assets/Water.png");
    sb_push(paths, "Assets/Sand.png");
    sb_push(paths, "Assets/Crate.png");
    sb_push(paths, "Assets/StoneBrick.png");

    Texture* diffuseArray = Malloc<Texture>();
    LoadTextureArray(diffuseArray, paths, true);
    SetAsset(state, ASSET_DIFFUSE_ARRAY, diffuseArray);

    sb_free(paths);

    Texture* crosshairTex = Malloc<Texture>();
    LoadTexture(crosshairTex, "Assets/Crosshair.png");
    SetAsset(state, ASSET_CROSSHAIR, crosshairTex);

    SoundAsset* stoneSound = Calloc<SoundAsset>();
    SetAsset(state, ASSET_STONE_SOUND, stoneSound);
    LoadSound(stoneSound, "Assets/Stone.ogg");

    SoundAsset* leavesSound = Calloc<SoundAsset>();
    SetAsset(state, ASSET_LEAVES_SOUND, leavesSound);
    LoadSound(leavesSound, "Assets/Leaves.ogg");

    MusicAsset* music = Calloc<MusicAsset>();
    SetAsset(state, ASSET_MUSIC, music);
    LoadMusic(music, "Assets/LittleTown.ogg");
    state->currentMusic = music;

    PlayMusic(music);
    ChangeVolume(state, 75.0f, 5.0f);
}
