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

static inline char* GetBlockName(World* world, Block block)
{
    return world->blockData[block].name;
}

static inline uint8_t GetBlockAlpha(World* world, Block block)
{
    return world->blockData[block].alpha;
}

static inline BlockSurface GetBlockSurface(World* world, Block block)
{
    return world->blockData[block].surface;
}

static inline BlockCollider GetBlockCollider(World* world, Block block)
{
    return world->blockData[block].collider;
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

static void CreateBlockData(GameState* state, BlockData* data)
{
    BlockData& air = data[BLOCK_AIR];
    air.cull = CULL_ALL;
    air.passable = true;
    air.onSetSound = GetSound(state, SOUND_LEAVES);
    air.lightStep = 1;
    air.name = "Air";

    BlockData& grass = data[BLOCK_GRASS];
    SetBlockTextures(grass, IMAGE_GRASS, IMAGE_DIRT, IMAGE_GRASS_SIDE);
    grass.buildFunc = BuildBlock;
    grass.onSetSound = GetSound(state, SOUND_STONE);
    grass.lightStep = INT_MAX;
    grass.name = "Grass";
    
    BlockData& dirt = data[BLOCK_DIRT];
    SetBlockTextures(dirt, IMAGE_DIRT);
    dirt.buildFunc = BuildBlock;
    dirt.onSetSound = GetSound(state, SOUND_STONE);
    dirt.lightStep = INT_MAX;
    dirt.name = "Dirt";

    BlockData& stone = data[BLOCK_STONE];
    SetBlockTextures(stone, IMAGE_STONE);
    stone.buildFunc = BuildBlock;
    stone.onSetSound = GetSound(state, SOUND_STONE);
    stone.lightStep = INT_MAX;
    stone.name = "Stone";

    BlockData& water = data[BLOCK_WATER];
    SetBlockTextures(water, IMAGE_WATER);
    water.meshType = MESH_TRANSPARENT;
    water.buildFunc = BuildBlock;
    water.cull = CULL_TRANSPARENT;
    water.passable = true;
    water.onSetSound = GetSound(state, SOUND_STONE);
    water.lightStep = 2;
    water.alpha = 127;
    water.name = "Water";

    BlockData& sand = data[BLOCK_SAND];
    SetBlockTextures(sand, IMAGE_SAND);
    sand.buildFunc = BuildBlock;
    sand.onSetSound = GetSound(state, SOUND_STONE);
    sand.lightStep = INT_MAX;
    sand.name = "Sand";

    BlockData& crate = data[BLOCK_CRATE];
    SetBlockTextures(crate, IMAGE_CRATE);
    crate.buildFunc = BuildBlock;
    crate.onSetSound = GetSound(state, SOUND_STONE);
    crate.lightStep = INT_MAX;
    crate.name = "Crate";

    BlockData& stoneBrick = data[BLOCK_STONE_BRICK];
    SetBlockTextures(stoneBrick, IMAGE_STONE_BRICK);
    stoneBrick.buildFunc = BuildBlock;
    stoneBrick.onSetSound = GetSound(state, SOUND_STONE);
    stoneBrick.lightStep = INT_MAX;
    stoneBrick.name = "Stone Brick";

    BlockData& metalCrate = data[BLOCK_METAL_CRATE];
    SetBlockTextures(metalCrate, IMAGE_METAL_CRATE);
    metalCrate.buildFunc = BuildBlock;
    metalCrate.onSetSound = GetSound(state, SOUND_STONE);
    metalCrate.lightStep = INT_MAX;
    metalCrate.name = "Metal Crate";

    BlockData& wood = data[BLOCK_WOOD];
    SetBlockTextures(wood, IMAGE_WOOD_TOP, IMAGE_WOOD_TOP, IMAGE_WOOD);
    wood.buildFunc = BuildBlock;
    wood.onSetSound = GetSound(state, SOUND_STONE);
    wood.lightStep = INT_MAX;
    wood.name = "Wood";

    BlockData& leaves = data[BLOCK_LEAVES];
    SetBlockTextures(leaves, IMAGE_LEAVES);
    leaves.buildFunc = BuildBlock;
    leaves.onSetSound = GetSound(state, SOUND_STONE);
    leaves.lightStep = INT_MAX;
    leaves.name = "Leaves";

    BlockData& clay = data[BLOCK_CLAY];
    SetBlockTextures(clay, IMAGE_CLAY);
    clay.buildFunc = BuildBlock;
    clay.onSetSound = GetSound(state, SOUND_STONE);
    clay.lightStep = INT_MAX;
    clay.name = "Clay";

    BlockData& snow = data[BLOCK_SNOW];
    SetBlockTextures(snow, IMAGE_SNOW, IMAGE_DIRT, IMAGE_SNOW_SIDE);
    snow.buildFunc = BuildBlock;
    snow.onSetSound = GetSound(state, SOUND_STONE);
    snow.lightStep = INT_MAX;
    snow.name = "Snow";

    BlockData& ice = data[BLOCK_ICE];
    SetBlockTextures(ice, IMAGE_ICE);
    ice.buildFunc = BuildBlock;
    ice.onSetSound = GetSound(state, SOUND_STONE);
    ice.surface = SURFACE_ICE;
    ice.lightStep = INT_MAX;
    ice.name = "Ice";

    BlockData& lantern = data[BLOCK_LANTERN];
    SetBlockTextures(lantern, IMAGE_LANTERN_ON);
    lantern.buildFunc = BuildBlock;
    lantern.onSetSound = GetSound(state, SOUND_STONE);
    lantern.lightStep = 1;
    lantern.lightEmitted = MAX_LIGHT;
    lantern.name = "Lantern";

    BlockData& trampoline = data[BLOCK_TRAMPOLINE];
    SetBlockTextures(trampoline, IMAGE_TRAMPOLINE);
    trampoline.buildFunc = BuildBlock;
    trampoline.onSetSound = GetSound(state, SOUND_STONE);
    trampoline.collider = COLLIDER_BOUNCE;
    trampoline.lightStep = INT_MAX;
    trampoline.name = "Trampoline";

    BlockData& cactus = data[BLOCK_CACTUS];
    SetBlockTextures(cactus, IMAGE_CACTUS_TOP, IMAGE_CACTUS_BOTTOM, IMAGE_CACTUS_SIDE);
    cactus.buildFunc = BuildBlock;
    cactus.onSetSound = GetSound(state, SOUND_STONE);
    cactus.lightStep = INT_MAX;
    cactus.name = "Cactus";

    BlockData& magma = data[BLOCK_MAGMA];
    SetBlockTextures(magma, IMAGE_MAGMA);
    magma.buildFunc = BuildBlock;
    magma.onSetSound = GetSound(state, SOUND_STONE);
    magma.lightStep = 1;
    magma.lightEmitted = 10;
    magma.name = "Magma";

    BlockData& cooledMagma = data[BLOCK_COOLED_MAGMA];
    SetBlockTextures(cooledMagma, IMAGE_COOLED_MAGMA);
    cooledMagma.buildFunc = BuildBlock;
    cooledMagma.onSetSound = GetSound(state, SOUND_STONE);
    cooledMagma.lightStep = INT_MAX;
    cooledMagma.name = "Cooled Magma";

    BlockData& obsidian = data[BLOCK_OBSIDIAN];
    SetBlockTextures(obsidian, IMAGE_OBSIDIAN);
    obsidian.buildFunc = BuildBlock;
    obsidian.onSetSound = GetSound(state, SOUND_STONE);
    obsidian.lightStep = INT_MAX;
    obsidian.name = "Obsidian";
}
