// Voxel Engine
// Jason Bricco

#define MESH_PARAMS 10
#define CAMERA_FOV 45.0f
#define WORLD_UP vec3(0.0f, 1.0f, 0.0f)

enum ShaderType
{
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    SHADER_PROGRAM
};

struct Graphic
{
    vec2 pos;
    Mesh2D mesh;
    Texture texture;
    Shader shader;
};

enum FrustumVisibility
{
    FRUSTUM_INVISIBLE,
    FRUSTUM_VISIBLE,
    FRUSTUM_PARTIAL
};

// Define a plane as having a point, p, and a normal, n.
struct Plane
{
    vec3 p, n;
};

struct Camera
{
    vec3 pos, target, up;
    vec3 forward, right;

    // Distance to the near and far frustum planes.
    float nearDist, farDist;

    // Width and height values for the near and far frustum planes.
    float nearW, farW;
    float nearH, farH;

    // Rotation angles in degrees.
    float yaw, pitch;

    float sensitivity;

    Plane planes[6];
};

struct MeshList
{
    Mesh** meshes;
    int count;
    int maxCount;

    MeshList()
    {
        meshes = Malloc(Mesh*, sizeof(Mesh*) * 512, "MeshList");
        count = 0;
        maxCount = 512;
    }

    inline void AddMesh(Mesh* mesh)
    {
        if (count + 1 > maxCount)
        {
            int newSize = maxCount * 2;
            meshes = (Mesh**)realloc(meshes, sizeof(Mesh*) * newSize);
            maxCount = newSize;
        }

        meshes[count++] = mesh;
    }

    inline Mesh* GetMesh(int i)
    {
        return meshes[i];
    }

    inline void Reset()
    {
        count = 0;
    }
};

struct Renderer
{
    Camera* camera;

    int windowWidth, windowHeight;

    // Shader uniforms.
    GLint view_0, model_0, proj_0;
    GLint view_1, model_1, proj_1, time_1;

    mat4 perspective, ortho, view;

    MeshList meshLists[CHUNK_MESH_COUNT];

    Graphic* crosshair;

    // Time for shader animation.
    float animTime;
};
