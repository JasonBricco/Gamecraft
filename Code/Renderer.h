//
// Gamecraft
//

#define PIXELS_PER_UNIT 32.0f
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
    Mesh2D mesh;
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
    Mesh& mesh;
    vec3 pos;
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

enum FadePriority
{
    FADE_PRIORITY_NONE,
    FADE_PRIORITY_FLUID,
    FADE_PRIORITY_DAMAGE
};

struct Renderer
{
    mat4 perspective;

    vector<ChunkMesh> meshLists[MESH_TYPE_COUNT];
    vector<ParticleEmitter*> emitters;

    BlockAnimation blockAnimation[MESH_TYPE_COUNT];

    Graphic* crosshair;

    // Screen fading.
    Color prevFadeColor, fadeColor;
    FadePriority prevFadePriority, fadePriority;
    Shader* fadeShader;
    Mesh2D fadeMesh;
    float fadeTimeLeft;

    // Backface culling will be enabled if this is true.
    bool disableFluidCull;

    float ambient;
    vec3 clearColor;

    bool fadeEnv;
    LerpData<vec3> clearColorLerp;
    LerpData<float> ambientLerp;

    // Antialiasing.
    int samplesAA;
    GLuint colAA, depthAA;
    GLuint fboAA;

    ObjectPool<MeshData> meshData;
    ObjectPool<MeshData2D> meshData2D;
};

static void LoadShader(Shader* shader, int vertLength, char* vertCode, int fragLength, char* fragCode);
static Texture LoadTexture(int width, int height, uint8_t* pixels);
static Texture LoadTextureArray(ImageData* data, int arrayCount, int total, uint8_t* assetData);

static inline void SetUniform(GLint loc, GLfloat f);
static inline void SetUniform(GLint loc, vec2 v);
static inline void SetUniform(GLint loc, vec3 v);
static inline void SetUniform(GLint loc, vec4 v);
static inline void SetUniform(GLint loc, mat4 m);

static bool ShaderHasErrors(GLuint shader, ShaderType type);
