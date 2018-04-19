// Voxel Engine
// Jason Bricco

#define CAMERA_FOV 45.0f

struct Mesh
{
	GLuint vb, ib, va;

	vector<float> vertices;
	vector<int> indices;
};

// Standard first-person camera.
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
	GLFWwindow* window;
	Camera* camera;

	int windowWidth, windowHeight;

	vec3 worldUp;

	unordered_map<string, GLint> uniforms;
	int paramCount;

	mat4 projection;
	mat4 view;

	GLuint texture;
	GLuint programs[1];

	ivec3 cursor;
};
