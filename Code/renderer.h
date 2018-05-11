// Voxel Engine
// Jason Bricco

#define MESH_PARAMS 10
#define CAMERA_FOV 45.0f
#define WORLD_UP vec3(0.0f, 1.0f, 0.0f)

struct Graphic
{
	GLuint vb, ib, va;
	int indices[6];
	float vertices[16];
	int shaderID;
	GLuint texture;
	vec2 pos;
};

struct Camera
{
	vec3 pos, target, up;
	vec3 forward, right;

	// Angles in degrees.
	float yaw, pitch;

	float sensitivity;
};

struct Renderer
{
	Camera* camera;

	int windowWidth, windowHeight;

	unordered_map<string, GLint> uniforms;

	mat4 perspective, ortho, view;

	Graphic* crosshair;

	// Textures.
	GLuint blockTextures;

	GLuint programs[2];
};
