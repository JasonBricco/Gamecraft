//
// Jason Bricco
//

typedef uint16_t Block;

using BuildBlockFunc = void(*)(World*, Chunk*, Mesh*, int, int, int, Block);

struct BlockData
{
    float textures[6];
    bool passable;
    BlockMeshType meshType;
    CullType cull;
    BuildBlockFunc buildFunc;
};
