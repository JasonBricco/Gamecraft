//
// Gamecraft
//

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
    vec3 pos;
    Mesh mesh;
    Texture texture;
    Shader* shader;
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

struct ChunkMesh
{
    Mesh mesh;
    vec3 pos;
};

struct ChunkMeshList
{
    ChunkMesh* meshes;
    int count, max;
};

struct Camera
{
    vec3 pos, target, up;
    vec3 forward, right;

    mat4 view;

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

struct Renderer
{
    // Shader uniforms.
    GLint view_0, model_0, proj_0;
    GLint view_1, model_1, proj_1, time_1;

    mat4 perspective;

    ChunkMeshList meshLists[MESH_TYPE_COUNT];

    Graphic* crosshair;

    // Screen fading.
    Color fadeColor;
    Shader* fadeShader;
    Mesh fadeMesh;

    // Backface culling will be enabled if this is true.
    bool disableFluidCull;

    // Time for shader animation.
    float animTime;

    // Antialiasing.
    int samplesAA;
    GLuint colAA, depthAA;
    GLuint fboAA;

    ObjectPool<MeshData> meshData;
    CRITICAL_SECTION meshCS;
};

static void LoadShader(Shader* shader, int vertLength, char* vertCode, int fragLength, char* fragCode);
static Texture LoadTexture(int width, int height, uint8_t* pixels);
static Texture LoadTextureArray(ImageData* data, int count, char* assetData);

static inline void SetUniform(GLint loc, GLfloat f);
static inline void SetUniform(GLint loc, vec2 v);
static inline void SetUniform(GLint loc, vec3 v);
static inline void SetUniform(GLint loc, vec4 v);
static inline void SetUniform(GLint loc, mat4 m);
