//
// Gamecraft
//

struct ParticleEmitter;

using ParticleFunc = void(*)(ParticleEmitter& emitter, World* world, float deltaTime);

struct Particle
{
	vec3 pos, wPos;
	vec3 accel, velocity;
	float timeLeft;
};

struct ParticleEmitter
{
	bool active;
	vec3 pos;
	Particle* particles;
	int count, spawnCount;
	int maxParticles;
	float lifetime, timePerSpawn, timer;
	float radius;
	ImageID image;
	Mesh2D mesh;
	GLuint modelBuffer;
	ParticleFunc update;
};

static void DrawParticles(GameState* state, ParticleEmitter& emitter, Camera* cam);
