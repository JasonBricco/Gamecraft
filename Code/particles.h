//
// Gamecraft
//

#define MAX_PARTICLES 8192

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
	Particle particles[MAX_PARTICLES];
	int count;
	int spawnCount;
	float lifeTime, timer;
	float radius;
	Mesh mesh;
	GLuint modelBuffer;
};

static void DrawParticles(GameState* state, ParticleEmitter& emitter, Camera* cam, ImageID image);
