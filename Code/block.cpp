//
// Jason Bricco
//

static BlockData g_blockData[BLOCK_COUNT];

static inline float* GetTextures(Block block)
{
    return g_blockData[block].textures;
}

static inline CullType GetCullType(Block block)
{
    return g_blockData[block].cull;
}

static inline bool IsPassable(Block block)
{
    return g_blockData[block].passable;
}

static inline BlockMeshType GetMeshType(Block block)
{
    return g_blockData[block].meshType;
}

static inline BuildBlockFunc BuildFunc(Block block)
{
    return g_blockData[block].buildFunc;
}

static inline SoundAsset* GetBlockSetSound(Block block)
{
    return g_blockData[block].onSetSound;
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

static void CreateBlockData()
{
    BlockData& air = g_blockData[BLOCK_AIR];
    air.cull = CULL_ALL;
    air.passable = true;
    air.onSetSound = GetAsset<SoundAsset>(ASSET_LEAVES_SOUND);

    BlockData& grass = g_blockData[BLOCK_GRASS];
    SetBlockTextures(grass, 0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    grass.buildFunc = BuildBlock;
    grass.onSetSound = GetAsset<SoundAsset>(ASSET_STONE_SOUND);
    
    BlockData& dirt = g_blockData[BLOCK_DIRT];
    SetBlockTextures(dirt, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f);
    dirt.buildFunc = BuildBlock;
    dirt.onSetSound = GetAsset<SoundAsset>(ASSET_STONE_SOUND);

    BlockData& stone = g_blockData[BLOCK_STONE];
    SetBlockTextures(stone, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f);
    stone.buildFunc = BuildBlock;
    stone.onSetSound = GetAsset<SoundAsset>(ASSET_STONE_SOUND);

    BlockData& water = g_blockData[BLOCK_WATER];
    SetBlockTextures(water, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f);
    water.meshType = MESH_TYPE_FLUID;
    water.buildFunc = BuildBlock;
    water.cull = CULL_TRANSPARENT;
    water.passable = true;
    water.onSetSound = GetAsset<SoundAsset>(ASSET_STONE_SOUND);

    BlockData& sand = g_blockData[BLOCK_SAND];
    SetBlockTextures(sand, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f);
    sand.buildFunc = BuildBlock;
    sand.onSetSound = GetAsset<SoundAsset>(ASSET_STONE_SOUND);

    BlockData& crate = g_blockData[BLOCK_CRATE];
    SetBlockTextures(crate, 6.0f, 6.0f, 6.0f, 6.0f, 6.0f, 6.0f);
    crate.buildFunc = BuildBlock;
    crate.onSetSound = GetAsset<SoundAsset>(ASSET_STONE_SOUND);

    BlockData& stoneBrick = g_blockData[BLOCK_STONEBRICK];
    SetBlockTextures(stoneBrick, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f);
    stoneBrick.buildFunc = BuildBlock;
    stoneBrick.onSetSound = GetAsset<SoundAsset>(ASSET_STONE_SOUND);
}

#define g_blockData Error_Invalid_Use
