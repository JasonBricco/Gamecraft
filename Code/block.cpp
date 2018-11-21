//
// Jason Bricco
//

static inline float* GetTextures(World* world, Block block)
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

static inline BlockMeshType GetMeshType(World* world, Block block)
{
    return world->blockData[block].meshType;
}

static inline BuildBlockFunc BuildFunc(World* world, Block block)
{
    return world->blockData[block].buildFunc;
}

static inline SoundAsset* GetBlockSetSound(World* world, Block block)
{
    return world->blockData[block].onSetSound;
}

static inline uint8_t GetLightEmitted(World* world, Block block)
{
    return world->blockData[block].light;
}

static inline int GetBlockLightStep(World* world, Block block)
{
    return world->blockData[block].lightStep;
}

static inline bool IsTransparent(World* world, Block block)
{
    return GetCullType(world, block) != CULL_OPAQUE;
}

static inline void SetBlockTextures(BlockData& data, float top, float bottom, float front, float back, float right, float left)
{
    data.textures[FACE_TOP] = top;
    data.textures[FACE_BOTTOM] = bottom;
    data.textures[FACE_FRONT] = front;
    data.textures[FACE_BACK] = back;
    data.textures[FACE_RIGHT] = right;
    data.textures[FACE_LEFT] = left;
}

static void CreateBlockData(GameState* state, BlockData* data)
{
    BlockData& air = data[BLOCK_AIR];
    air.cull = CULL_ALL;
    air.passable = true;
    air.onSetSound = GetAsset<SoundAsset>(state, ASSET_LEAVES_SOUND);
    air.lightStep = 1;

    BlockData& grass = data[BLOCK_GRASS];
    SetBlockTextures(grass, 0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    grass.buildFunc = BuildBlock;
    grass.onSetSound = GetAsset<SoundAsset>(state, ASSET_STONE_SOUND);
    
    BlockData& dirt = data[BLOCK_DIRT];
    SetBlockTextures(dirt, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f);
    dirt.buildFunc = BuildBlock;
    dirt.onSetSound = GetAsset<SoundAsset>(state, ASSET_STONE_SOUND);

    BlockData& stone = data[BLOCK_STONE];
    SetBlockTextures(stone, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f);
    stone.buildFunc = BuildBlock;
    stone.onSetSound = GetAsset<SoundAsset>(state, ASSET_STONE_SOUND);

    BlockData& water = data[BLOCK_WATER];
    SetBlockTextures(water, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f);
    water.meshType = MESH_TYPE_FLUID;
    water.buildFunc = BuildBlock;
    water.cull = CULL_TRANSPARENT;
    water.passable = true;
    water.onSetSound = GetAsset<SoundAsset>(state, ASSET_STONE_SOUND);
    water.lightStep = 2;

    BlockData& sand = data[BLOCK_SAND];
    SetBlockTextures(sand, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f);
    sand.buildFunc = BuildBlock;
    sand.onSetSound = GetAsset<SoundAsset>(state, ASSET_STONE_SOUND);

    BlockData& crate = data[BLOCK_CRATE];
    SetBlockTextures(crate, 6.0f, 6.0f, 6.0f, 6.0f, 6.0f, 6.0f);
    crate.buildFunc = BuildBlock;
    crate.onSetSound = GetAsset<SoundAsset>(state, ASSET_STONE_SOUND);

    BlockData& stoneBrick = data[BLOCK_STONEBRICK];
    SetBlockTextures(stoneBrick, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f);
    stoneBrick.buildFunc = BuildBlock;
    stoneBrick.onSetSound = GetAsset<SoundAsset>(state, ASSET_STONE_SOUND);
}
