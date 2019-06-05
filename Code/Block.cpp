//
// Gamecraft
//

static inline uint16_t* GetTextures(World* world, Block block)
{
    return world->blockData[block].textures;
}

static inline CullType GetCullType(World* world, Block block)
{
    return world->blockData[block].cull;
}

static inline bool IsPassable(World* world, Block block)
{
    return world->blockData[block].passable;
}

static inline bool IsVisible(World* world, Block block)
{
    return !world->blockData[block].invisible;
}

static inline BlockMeshType GetMeshType(World* world, Block block)
{
    return world->blockData[block].meshType;
}

static inline BuildBlockFunc BuildFunc(World* world, Block block)
{
    return world->blockData[block].buildFunc;
}

static inline Sound GetSetSound(World* world, Block block)
{
    return world->blockData[block].onSetSound;
}

static inline bool IsTransparent(World* world, Block block)
{
    return GetCullType(world, block) != CULL_OPAQUE;
}

static inline char* GetName(World* world, Block block)
{
    return world->blockData[block].name;
}

static inline uint8_t GetAlpha(World* world, Block block)
{
    return world->blockData[block].alpha;
}

static inline BlockSurface GetSurface(World* world, Block block)
{
    return world->blockData[block].surface;
}

static inline BlockCollideFunc GetCollideFunc(World* world, Block block)
{
    return world->blockData[block].collideFunc;
}

static inline bool IsOpaque(World* world, Block block)
{
    return world->blockData[block].lightStep == INT_MAX;
}

static inline int GetLightStep(World* world, Block block)
{
    return world->blockData[block].lightStep;
}

static inline int GetLightEmitted(World* world, Block block)
{
    return world->blockData[block].lightEmitted;
}

static inline OverTimeDamage GetOverTimeDamage(World* world, Block block)
{
    return world->blockData[block].overTimeDamage;
}

static inline bool IsFluid(World* world, Block block)
{
    return world->blockData[block].isFluid;
}

static inline Color GetScreenTint(World* world, Block block)
{
    return world->blockData[block].tint;
}

static inline void SetBlockTextures(BlockData& data, uint16_t top, uint16_t bottom, uint16_t front, uint16_t back, uint16_t right, uint16_t left)
{
    data.textures[FACE_TOP] = top;
    data.textures[FACE_BOTTOM] = bottom;
    data.textures[FACE_FRONT] = front;
    data.textures[FACE_BACK] = back;
    data.textures[FACE_RIGHT] = right;
    data.textures[FACE_LEFT] = left;
}

static inline void SetBlockTextures(BlockData& data, uint16_t tex)
{
    SetBlockTextures(data, tex, tex, tex, tex, tex, tex);
}

static inline void SetBlockTextures(BlockData& data, uint16_t top, uint16_t bottom, uint16_t sides)
{
    SetBlockTextures(data, top, bottom, sides, sides, sides, sides);
}

static int ComputeAnimationFrame(BlockAnimation& anim, float deltaTime)
{
    anim.timeLeft -= deltaTime;

    if (anim.timeLeft <= 0.0f)
    {
        anim.index = (anim.index + 1) % anim.frames.size();
        anim.timeLeft = 1.0f / anim.framesPerSec;
    }

    return anim.frames[anim.index];
}

static inline BlockData& CreateDefaultBlock(GameState* state, BlockData* dataArray, BlockType type)
{
    BlockData& data = dataArray[type];
    data.buildFunc = BuildBlock;
    data.onSetSound = GetSound(state, SOUND_STONE);
    data.collideFunc = DefaultCollideFunc;
    data.lightStep = INT_MAX;
    return data;
}

static void CreateBlockData(GameState* state, BlockData* data)
{
    BlockData& air = CreateDefaultBlock(state, data, BLOCK_AIR);
    air.invisible = true;
    air.cull = CULL_ALL;
    air.passable = true;
    air.onSetSound = GetSound(state, SOUND_LEAVES);
    air.lightStep = 1;
    air.collideFunc = IgnoreCollideFunc;
    air.name = "Air";

    BlockData& grass = CreateDefaultBlock(state, data, BLOCK_GRASS);
    SetBlockTextures(grass, IMAGE_GRASS, IMAGE_DIRT, IMAGE_GRASS_SIDE);
    grass.name = "Grass";
    
    BlockData& dirt = CreateDefaultBlock(state, data, BLOCK_DIRT);
    SetBlockTextures(dirt, IMAGE_DIRT);
    dirt.name = "Dirt";

    BlockData& stone = CreateDefaultBlock(state, data, BLOCK_STONE);
    SetBlockTextures(stone, IMAGE_STONE);
    stone.name = "Stone";

    BlockData& water = CreateDefaultBlock(state, data, BLOCK_WATER);
    SetBlockTextures(water, IMAGE_WATER);
    water.meshType = MESH_FLUID;
    water.cull = CULL_TRANSPARENT;
    water.passable = true;
    water.lightStep = 2;
    water.alpha = 127;
    water.isFluid = true;
    water.tint = Color(0.17f, 0.45f, 0.69f, 0.75f);
    water.name = "Water";

    BlockData& sand = CreateDefaultBlock(state, data, BLOCK_SAND);
    SetBlockTextures(sand, IMAGE_SAND);
    sand.name = "Sand";

    BlockData& crate = CreateDefaultBlock(state, data, BLOCK_CRATE);
    SetBlockTextures(crate, IMAGE_CRATE);
    crate.name = "Crate";

    BlockData& stoneBrick = CreateDefaultBlock(state, data, BLOCK_STONE_BRICK);
    SetBlockTextures(stoneBrick, IMAGE_STONE_BRICK);
    stoneBrick.name = "Stone Brick";

    BlockData& metalCrate = CreateDefaultBlock(state, data, BLOCK_METAL_CRATE);
    SetBlockTextures(metalCrate, IMAGE_METAL_CRATE);
    metalCrate.name = "Metal Crate";

    BlockData& wood = CreateDefaultBlock(state, data, BLOCK_WOOD);
    SetBlockTextures(wood, IMAGE_WOOD_TOP, IMAGE_WOOD_TOP, IMAGE_WOOD);
    wood.name = "Wood";

    BlockData& leaves = CreateDefaultBlock(state, data, BLOCK_LEAVES);
    SetBlockTextures(leaves, IMAGE_LEAVES);
    leaves.name = "Leaves";

    BlockData& clay = CreateDefaultBlock(state, data, BLOCK_CLAY);
    SetBlockTextures(clay, IMAGE_CLAY);
    clay.name = "Clay";

    BlockData& snow = CreateDefaultBlock(state, data, BLOCK_SNOW);
    SetBlockTextures(snow, IMAGE_SNOW, IMAGE_DIRT, IMAGE_SNOW_SIDE);
    snow.name = "Snow";

    BlockData& ice = CreateDefaultBlock(state, data, BLOCK_ICE);
    SetBlockTextures(ice, IMAGE_ICE);
    ice.surface = SURFACE_ICE;
    ice.name = "Ice";

    BlockData& lantern = CreateDefaultBlock(state, data, BLOCK_LANTERN);
    SetBlockTextures(lantern, IMAGE_LANTERN_ON);
    lantern.lightStep = 1;
    lantern.lightEmitted = MAX_LIGHT;
    lantern.name = "Lantern";

    BlockData& trampoline = CreateDefaultBlock(state, data, BLOCK_TRAMPOLINE);
    SetBlockTextures(trampoline, IMAGE_TRAMPOLINE);
    trampoline.collideFunc = BounceCollideFunc;
    trampoline.name = "Trampoline";

    BlockData& cactus = CreateDefaultBlock(state, data, BLOCK_CACTUS);
    SetBlockTextures(cactus, IMAGE_CACTUS_TOP, IMAGE_CACTUS_BOTTOM, IMAGE_CACTUS_SIDE);
    cactus.overTimeDamage = { BLOCK_CACTUS, 1.0f, 1 };
    cactus.collideFunc = OverTimeDamageCollideFunc;
    cactus.name = "Cactus";

    BlockData& magma = CreateDefaultBlock(state, data, BLOCK_MAGMA);
    SetBlockTextures(magma, IMAGE_MAGMA);
    magma.meshType = MESH_MAGMA;
    magma.lightStep = 1;
    magma.lightEmitted = 10;
    magma.overTimeDamage = { BLOCK_MAGMA, 0.5f, 1 };
    magma.collideFunc = OverTimeDamageCollideFunc;
    magma.name = "Magma";

    BlockData& cooledMagma = CreateDefaultBlock(state, data, BLOCK_COOLED_MAGMA);
    SetBlockTextures(cooledMagma, IMAGE_COOLED_MAGMA);
    cooledMagma.name = "Cooled Magma";

    BlockData& obsidian = CreateDefaultBlock(state, data, BLOCK_OBSIDIAN);
    SetBlockTextures(obsidian, IMAGE_OBSIDIAN);
    obsidian.name = "Obsidian";

    BlockData& killZone = CreateDefaultBlock(state, data, BLOCK_KILL_ZONE);
    killZone.invisible = true;
    killZone.cull = CULL_ALL;
    killZone.lightStep = 1;
    killZone.collideFunc = KillCollideFunc;
    killZone.name = "Kill Zone";

    BlockData& lava = CreateDefaultBlock(state, data, BLOCK_LAVA);
    SetBlockTextures(lava, IMAGE_LAVA);
    lava.meshType = MESH_FLUID;
    lava.cull = CULL_TRANSPARENT;
    lava.passable = true;
    lava.lightStep = 1;
    lava.lightEmitted = 10;
    lava.alpha = 127;
    lava.isFluid = true;
    lava.tint = Color(0.75f, 0.16f, 0.0f, 0.75f);
    lava.name = "Lava";

    Renderer& rend = state->renderer;

    BlockAnimation& opaqueAnim = rend.blockAnimation[MESH_OPAQUE];
    opaqueAnim.frames = { 0 };

    BlockAnimation& transparentAnim = rend.blockAnimation[MESH_TRANSPARENT];
    transparentAnim.frames = { 0 };

    BlockAnimation& fluidAnim = rend.blockAnimation[MESH_FLUID];
    fluidAnim.framesPerSec = 2.0f;
    fluidAnim.frames = { 0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1 };

    BlockAnimation& magmaAnim = rend.blockAnimation[MESH_MAGMA];
    magmaAnim.framesPerSec = 4.0f;
    magmaAnim.frames = 
    { 
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
    };

    for (int i = 0; i < MESH_TYPE_COUNT; i++)
        rend.blockAnimation[i].timeLeft = 1.0f / rend.blockAnimation[i].framesPerSec;
}
