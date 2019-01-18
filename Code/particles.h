//
// Jason Bricco
//

#define MAX_PARTICLES 2048

struct Particle
{
	vec3 pos;
	vec3 velocity;
};

struct ParticleEmitter
{
	bool active;
	vec3 pos;
	Particle particles[MAX_PARTICLES];
	int count;
	int spawnCount;
	float timer;
	float radius;
	Mesh mesh;
	GLuint modelBuffer;
};

static void DrawParticles(GameState* state, ParticleEmitter& emitter, Camera* cam);
