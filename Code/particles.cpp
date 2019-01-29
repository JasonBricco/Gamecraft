//
// Jason Bricco
//

static void InitParticleEmitter(ParticleEmitter& emitter, int spawnCount, float radius)
{
	emitter.spawnCount = spawnCount;
	emitter.radius = radius;

	MeshData* data = CreateTempMeshData(4, 6); 

    SetIndices(data);
    SetUVs(data, 0.0f);

    data->positions[0] = vec3(-0.015625f, -0.125f, 0.015625f);
    data->positions[1] = vec3(-0.015625f, 0.125f, 0.015625f);
    data->positions[2] = vec3(0.015625f, 0.125f, 0.015625f);
    data->positions[3] = vec3(0.015625f, -0.125f, 0.015625f);
    data->vertexCount = 4;

    Mesh mesh = {};
    SetMeshFlags(mesh, MESH_NO_COLORS);
    
	glGenVertexArrays(1, &mesh.va);
	glBindVertexArray(mesh.va);

	FillMeshData(mesh, data, GL_STREAM_DRAW);

	for (int i = 0; i < 4; i++)
	{
		// The mat4 buffer takes up four locations in the shader, with the first starting at location 2.
		glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (GLvoid*)(sizeof(vec4) * i));
		glEnableVertexAttribArray(2 + i);
		glVertexAttribDivisor(2 + i, 1);
	}

	emitter.mesh = mesh;
}

static void SpawnParticle(ParticleEmitter& emitter, float x, float y, float z, vec3 velocity)
{
	int index = emitter.count;
	assert(index < MAX_PARTICLES);
	Particle particle = { vec3(x, y, z), velocity };
	emitter.particles[index] = particle;
	emitter.count++;
}

static void DestroyParticle(ParticleEmitter& emitter, int index)
{
	int end = emitter.count - 1;
	emitter.particles[index] = emitter.particles[end];

	emitter.count--;
}

static void UpdateParticles(ParticleEmitter& emitter, World* world, float deltaTime)
{
	emitter.timer -= deltaTime;

	if (emitter.active && emitter.timer <= 0.0f)
	{
		for (int i = 0; i < emitter.spawnCount; i++)
		{
			float x = emitter.pos.x + RandRange(-emitter.radius, emitter.radius);
			float z = emitter.pos.z + RandRange(-emitter.radius, emitter.radius);

			SpawnParticle(emitter, x, emitter.pos.y, z, vec3(0.0f, -30.0f, 0.0f));
		}

		emitter.timer = 0.01f;
	}

	// Particle simulation.
	for (int i = 0; i < emitter.count; i++)
	{
		Particle& particle = emitter.particles[i];
		particle.pos += (particle.velocity * deltaTime);

		Block block = GetBlock(world, BlockPos(particle.pos));

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
		matrices[i] = translate(mat4(1.0f), particle.pos);
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	Shader* shader = GetShader(state, SHADER_PARTICLE);

	UseShader(shader);
	SetUniform(shader->view, cam->view);
	SetUniform(shader->proj, cam->perspective);

	glBindTexture(GL_TEXTURE_2D, GetTexture(state, IMAGE_RAIN).id);

	glBindVertexArray(emitter.mesh.va);
	glDrawElementsInstanced(GL_TRIANGLES, emitter.mesh.indexCount, GL_UNSIGNED_INT, 0, emitter.count);
}
