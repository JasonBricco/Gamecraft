//
// Gamecraft
//

static void InitParticleEmitter(Renderer& rend, ParticleEmitter& emitter, int spawnCount, float radius)
{
	emitter.spawnCount = spawnCount;
	emitter.radius = radius;

	MeshData* data = GetMeshData(rend.meshData);
	assert(data != nullptr);
	
    SetIndices(data);
    SetUVs(data, 0);

    data->positions[0] = vec3(-0.015625f, -0.125f, 0.015625f);
    data->positions[1] = vec3(-0.015625f, 0.125f, 0.015625f);
    data->positions[2] = vec3(0.015625f, 0.125f, 0.015625f);
    data->positions[3] = vec3(0.015625f, -0.125f, 0.015625f);
    data->vertCount = 4;

	FillMeshData(rend.meshData, emitter.mesh, data, GL_STREAM_DRAW, MESH_NO_COLORS | MESH_NO_UVS);

	glGenBuffers(1, &emitter.modelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, emitter.modelBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * MAX_PARTICLES, NULL, GL_STREAM_DRAW);

	for (int i = 0; i < 4; i++)
	{
		// The mat4 buffer takes up four locations in the shader, with the first starting at location 2.
		glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (GLvoid*)(sizeof(vec4) * i));
		glEnableVertexAttribArray(2 + i);
		glVertexAttribDivisor(2 + i, 1);
	}
}

static void SpawnParticle(ParticleEmitter& emitter, float x, float y, float z, float lifeTime, vec3 velocity)
{
	Particle particle = { vec3(x, y, z), vec3(0.0f), velocity, lifeTime };
	emitter.particles[emitter.count++] = particle;
}

static void DestroyParticle(ParticleEmitter& emitter, int index)
{
	emitter.particles[index] = emitter.particles[emitter.count - 1];
	emitter.count--;
}

static void UpdateParticles(ParticleEmitter& emitter, World* world, float deltaTime)
{
	emitter.timer -= deltaTime;

	if (emitter.active && emitter.timer <= 0.0f)
	{
		for (int i = 0; i < emitter.spawnCount; i++)
		{
			float x = RandRange(-emitter.radius, emitter.radius);
			float z = RandRange(-emitter.radius, emitter.radius);

			SpawnParticle(emitter, x, emitter.pos.y, z, 10.0f, vec3(0.0f, -45.0f, 0.0f));
		}

		emitter.timer = 0.01f;
	}

	// Particle simulation.
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

	glBindTexture(GL_TEXTURE_2D, GetTexture(state, IMAGE_RAIN).id);

	glBindVertexArray(emitter.mesh.va);
	glDrawElementsInstanced(GL_TRIANGLES, emitter.mesh.indexCount, GL_UNSIGNED_SHORT, 0, emitter.count);
}
