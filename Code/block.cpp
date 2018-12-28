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

static inline Sound GetBlockSetSound(World* world, Block block)
{
    return world->blockData[block].onSetSound;
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

static inline void SetBlockTextures(BlockData& data, float tex)
{
    SetBlockTextures(data, tex, tex, tex, tex, tex, tex);
}

static inline void SetBlockTextures(BlockData& data, float top, float bottom, float sides)
{
    SetBlockTextures(data, top, bottom, sides, sides, sides, sides);
}

static void CreateBlockData(GameState* state, BlockData* data)
{
    BlockData& air = data[BLOCK_AIR];
    air.cull = CULL_ALL;
    air.passable = true;
    air.onSetSound = GetSound(state, SOUND_LEAVES);

    BlockData& grass = data[BLOCK_GRASS];
    SetBlockTextures(grass, IMAGE_GRASS, IMAGE_DIRT, IMAGE_GRASS_SIDE);
    grass.buildFunc = BuildBlock;
    grass.onSetSound = GetSound(state, SOUND_STONE);
    
    BlockData& dirt = data[BLOCK_DIRT];
    SetBlockTextures(dirt, IMAGE_DIRT);
    dirt.buildFunc = BuildBlock;
    dirt.onSetSound = GetSound(state, SOUND_STONE);

    BlockData& stone = data[BLOCK_STONE];
    SetBlockTextures(stone, IMAGE_STONE);
    stone.buildFunc = BuildBlock;
    stone.onSetSound = GetSound(state, SOUND_STONE);

    BlockData& water = data[BLOCK_WATER];
    SetBlockTextures(water, IMAGE_WATER);
    water.meshType = MESH_TYPE_FLUID;
    water.buildFunc = BuildBlock;
    water.cull = CULL_TRANSPARENT;
    water.passable = true;
    water.onSetSound = GetSound(state, SOUND_STONE);

    BlockData& sand = data[BLOCK_SAND];
    SetBlockTextures(sand, IMAGE_SAND);
    sand.buildFunc = BuildBlock;
    sand.onSetSound = GetSound(state, SOUND_STONE);

    BlockData& crate = data[BLOCK_CRATE];
    SetBlockTextures(crate, IMAGE_CRATE);
    crate.buildFunc = BuildBlock;
    crate.onSetSound = GetSound(state, SOUND_STONE);

    BlockData& stoneBrick = data[BLOCK_STONEBRICK];
    SetBlockTextures(stoneBrick, IMAGE_STONE_BRICK);
    stoneBrick.buildFunc = BuildBlock;
    stoneBrick.onSetSound = GetSound(state, SOUND_STONE);
}
