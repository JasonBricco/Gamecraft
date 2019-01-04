//
// Jason Bricco
//

static void InitParticleEmitter(ParticleEmitter& emitter, int spawnCount, float radius)
{
	emitter.spawnCount = spawnCount;
	emitter.radius = radius;

	Mesh* mesh = CreateMesh(20, 6);

	SetMeshIndices(mesh, 5);
	SetMeshVertex(mesh, -0.5f, -0.5f, 0.5f, 0.0f, 1.0f);
	SetMeshVertex(mesh, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f);
	SetMeshVertex(mesh, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f);
	SetMeshVertex(mesh, 0.5f, -0.5f, 0.5f, 1.0f, 1.0f);

	glGenVertexArrays(1, &mesh->va);
	glBindVertexArray(mesh->va);

	glGenBuffers(1, &mesh->vb);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mesh->vertCount, mesh->vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &mesh->ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * mesh->indexCount, mesh->indices, GL_STATIC_DRAW);

	glGenBuffers(1, &emitter.positions);
	glBindBuffer(GL_ARRAY_BUFFER, emitter.positions);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	int stride = 8 * sizeof(GLfloat);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(GLfloat))); 
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);

	emitter.mesh = mesh;
}

static void SpawnParticle(ParticleEmitter& emitter, float x, float y, float z, vec3 velocity)
{
	int index = emitter.count;
	assert(index < MAX_PARTICLES);
	emitter.particlePositions[index] = vec3(x, y, z);
	emitter.particleVelocity[index] = velocity;
	emitter.count++;
}

static void DestroyParticle(ParticleEmitter& emitter, int index)
{
	int end = emitter.count - 1;
	emitter.particlePositions[index] = emitter.particlePositions[end];
	emitter.particleVelocity[index] = emitter.particleVelocity[end];
	emitter.count--;
}

static void UpdateParticles(ParticleEmitter& emitter, World* world, float deltaTime)
{
	emitter.timer -= deltaTime;

	if (emitter.timer <= 0.0f)
	{
		for (int i = 0; i < emitter.spawnCount; i++)
		{
			float x = emitter.pos.x + RandRange(-emitter.radius, emitter.radius);
			float z = emitter.pos.z + RandRange(-emitter.radius, emitter.radius);

			SpawnParticle(emitter, x, emitter.pos.y, z, vec3(0.0f, -15.0f, 0.0f));
		}

		emitter.timer = 0.01f;
	}

	// Particle simulation.
	for (int i = 0; i < emitter.count; i++)
	{
		vec3& pos = emitter.particlePositions[i];
		vec3 velocity = emitter.particleVelocity[i];

		pos += (velocity * deltaTime);

		Block block = GetBlock(world, BlockPos(pos));

		if (!IsPassable(world, block))
			DestroyParticle(emitter, i);
	}
}

static void DrawParticles(GameState* state, ParticleEmitter& emitter, Camera* cam)
{
	glBindBuffer(GL_ARRAY_BUFFER, emitter.positions);
	
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, emitter.count * sizeof(GLfloat) * 3, emitter.particlePositions);

	Shader* shader = GetShader(state, SHADER_PARTICLE);

	UseShader(shader);
	SetUniform(shader->view, cam->view);
	SetUniform(shader->proj, cam->perspective);

	glBindTexture(GL_TEXTURE_2D, GetTexture(state, IMAGE_STONE).id);

	glBindVertexArray(emitter.mesh->va);
	glDrawElementsInstanced(GL_TRIANGLES, emitter.mesh->indexCount, GL_UNSIGNED_INT, 0, emitter.count);
}
