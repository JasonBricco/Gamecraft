//
// Jason Bricco
//

#define MAX_PARTICLES 4096

struct ParticleEmitter
{
	vec3 pos;
	vec3 particlePositions[MAX_PARTICLES];
	vec3 particleVelocity[MAX_PARTICLES];
	int count;
	int spawnCount;
	float timer;
	float radius;
	Mesh* mesh;
	GLuint positions;
};

static void DrawParticles(GameState* state, ParticleEmitter& emitter, Camera* cam);
