//
// Gamecraft
//

static void InitParticleEmitter(Renderer& rend, ParticleEmitter& emitter, int w, int h, int maxParticles)
{
	emitter.maxParticles = maxParticles;
	emitter.particles = new Particle[maxParticles];

	MeshData* data = GetMeshData(rend.meshData);
	assert(data != nullptr);
	
    SetIndices(data);
    SetUVs(data, 0);

    float halfW = (w / PIXELS_PER_UNIT) * 0.5f;
    float halfH = (h / PIXELS_PER_UNIT) * 0.5f;

    data->positions[0] = vec3(-halfW, -halfH, halfW);
    data->positions[1] = vec3(-halfW, halfH, halfW);
    data->positions[2] = vec3(halfW, halfH, halfW);
    data->positions[3] = vec3(halfW, -halfH, halfW);
    data->vertCount = 4;

	FillMeshData(rend.meshData, emitter.mesh, data, GL_STREAM_DRAW, MESH_NO_COLORS);

	glGenBuffers(1, &emitter.modelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, emitter.modelBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * emitter.maxParticles, NULL, GL_STREAM_DRAW);

	for (int i = 0; i < 4; i++)
	{
		// The mat4 buffer takes up four locations in the shader, with the first starting at location 2.
		glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (GLvoid*)(sizeof(vec4) * i));
		glEnableVertexAttribArray(2 + i);
		glVertexAttribDivisor(2 + i, 1);
	}
}

static void SetEmitterProperties(ParticleEmitter& emitter, ImageID image, int spawnCount, float radius, float lifeTime, ParticleFunc updateFunc)
{
	emitter.spawnCount = spawnCount;
	emitter.radius = radius;
	emitter.lifeTime = lifeTime;
	emitter.image = image;
	emitter.update = updateFunc;
}

static inline void DestroyParticle(ParticleEmitter& emitter, int index)
{
	emitter.particles[index] = emitter.particles[emitter.count - 1];
	emitter.count--;
}

static void DestroyAllParticles(ParticleEmitter& emitter)
{
	for (int i = emitter.count - 1; i >= 0; i--)
		DestroyParticle(emitter, i);
}

static void SpawnParticles(ParticleEmitter& emitter, vec3 accel, vec3 velocity, float deltaTime)
{
	emitter.timer -= deltaTime;

	if (emitter.active && emitter.timer <= 0.0f)
	{
		for (int i = 0; i < emitter.spawnCount; i++)
		{
			float x = RandRange(-emitter.radius, emitter.radius);
			float z = RandRange(-emitter.radius, emitter.radius);

			Particle particle;
			particle.pos = vec3(x, emitter.pos.y, z);
			particle.accel = accel;
			particle.velocity = velocity;
			particle.timeLeft = emitter.lifeTime;
			emitter.particles[emitter.count++] = particle;
			assert(emitter.count < emitter.maxParticles);
		}

		emitter.timer = 0.01f;
	}
}

static inline void MoveParticleLocal(Particle& particle, vec3 emitterP, float deltaTime)
{
	vec3 delta = particle.accel * 0.5f * Square(deltaTime) + particle.velocity * deltaTime;
	particle.velocity = particle.accel * deltaTime + particle.velocity;
	particle.pos += delta;
	particle.wPos = vec3(emitterP.x + particle.pos.x, particle.pos.y, emitterP.z + particle.pos.z);
}

static void UpdateRainParticles(ParticleEmitter& emitter, World* world, float deltaTime)
{
	SpawnParticles(emitter, vec3(0.0f, -15.0f, 0.0f), vec3(0.0f), deltaTime);

	for (int i = emitter.count - 1; i >= 0; i--)
	{
		Particle& particle = emitter.particles[i];
		particle.timeLeft -= deltaTime;

		if (particle.timeLeft <= 0.0f)
		{
			DestroyParticle(emitter, i);
			continue;
		}

		MoveParticleLocal(particle, emitter.pos, deltaTime);

		Block block = GetBlock(world, BlockPos(particle.wPos));

		if (!IsPassable(world, block))
			DestroyParticle(emitter, i);
	}
}

static void UpdateSnowParticles(ParticleEmitter& emitter, World* world, float deltaTime)
{
	SpawnParticles(emitter, vec3(0.0f), vec3(RandNormal() * 5.0f, -20.0f, RandNormal() * 5.0f), deltaTime);

	for (int i = emitter.count - 1; i >= 0; i--)
	{
		Particle& particle = emitter.particles[i];
		particle.timeLeft -= deltaTime;

		if (particle.timeLeft <= 0.0f)
		{
			DestroyParticle(emitter, i);
			continue;
		}

		particle.pos += (particle.velocity * deltaTime);
		particle.wPos = vec3(emitter.pos.x + particle.pos.x, particle.pos.y, emitter.pos.z + particle.pos.z);

		Block block = GetBlock(world, BlockPos(particle.wPos));

		if (!IsPassable(world, block))
			DestroyParticle(emitter, i);
	}
}

static void DrawParticles(GameState* state, ParticleEmitter& emitter, Camera* cam)
{
	if (emitter.count == 0) return;

	BEGIN_TIMED_BLOCK(DRAW_PARTICLES);

	glBindBuffer(GL_ARRAY_BUFFER, emitter.modelBuffer);
	mat4* matrices = (mat4*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	for (int i = 0; i < emitter.count; i++)
	{
		Particle& particle = emitter.particles[i];
		matrices[i] = translate(mat4(1.0f), particle.wPos);
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	Shader* shader = GetShader(state, SHADER_PARTICLE);

	UseShader(shader);
	SetUniform(shader->view, cam->view);
	SetUniform(shader->proj, state->renderer.perspective);

	glBindTexture(GL_TEXTURE_2D, GetTexture(state, emitter.image).id);

	glBindVertexArray(emitter.mesh.va);
	glDrawElementsInstanced(GL_TRIANGLES, emitter.mesh.indexCount, GL_UNSIGNED_SHORT, 0, emitter.count);

	END_TIMED_BLOCK(DRAW_PARTICLES);
}
