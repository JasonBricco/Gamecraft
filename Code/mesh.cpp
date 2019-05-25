//
// Gamecraft
//

static MeshData* GetMeshData(ObjectPool<MeshData>& pool)
{
	MeshData* meshData = pool.Get();
	meshData->vertCount = 0;

	for (int i = 0; i < MESH_TYPE_COUNT; i++)
		meshData->indices[i].count = 0;

	return meshData;
}

static MeshData2D* GetMeshData(ObjectPool<MeshData2D>& pool)
{
	return pool.Get();
}

static inline void SetIndices(MeshData* meshData, int index)
{
	uint16_t offset = (uint16_t)meshData->vertCount;
	int count = meshData->indices[index].count;

	meshData->indices[index].data[count] = offset + 2;
	meshData->indices[index].data[count + 1] = offset + 1;
	meshData->indices[index].data[count + 2] = offset;

	meshData->indices[index].data[count + 3] = offset + 3;
	meshData->indices[index].data[count + 4] = offset + 2;
	meshData->indices[index].data[count + 5] = offset;

	meshData->indices[index].count += 6;
}

static inline void SetIndices(MeshData2D* meshData)
{
	meshData->indices[0] = 2;
	meshData->indices[1] = 1;
	meshData->indices[2] = 0;

	meshData->indices[3] = 3;
	meshData->indices[4] = 2;
	meshData->indices[5] = 0;
}

static inline void SetUVs(MeshData* meshData, uint16_t w)
{
	int count = meshData->vertCount;
	meshData->uvs[count] = u16vec3(0, 1, w);
    meshData->uvs[count + 1] = u16vec3(0, 0, w);
    meshData->uvs[count + 2] = u16vec3(1, 0, w);
    meshData->uvs[count + 3] = u16vec3(1, 1, w);
}

static inline void SetUVs(MeshData2D* meshData)
{
	meshData->uvs[0] = u16vec2(0, 1);
    meshData->uvs[1] = u16vec2(0, 0);
    meshData->uvs[2] = u16vec2(1, 0);
    meshData->uvs[3] = u16vec2(1, 1);
}

static inline void SetAlpha(MeshData* meshData, uint8_t value)
{
	int count = meshData->vertCount;
	u8vec4* val = (u8vec4*)(meshData->alpha + count);
	*val = u8vec4(value);
}

static void FillMeshData(ObjectPool<MeshData>& pool, Mesh& mesh, MeshData* meshData, GLenum type)
{
	assert(meshData->vertCount > 0);

	glGenVertexArrays(1, &mesh.va);
	glBindVertexArray(mesh.va);

	// Vertex position buffer.
	glGenBuffers(1, &mesh.positions);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.positions);

	glBufferData(GL_ARRAY_BUFFER, sizeof(u8vec3) * meshData->vertCount, meshData->positions, type);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	// Vertex texture coordinates buffer.
	glGenBuffers(1, &mesh.uvs);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.uvs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(u16vec3) * meshData->vertCount, meshData->uvs, type);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL); 
	glEnableVertexAttribArray(1);

	// Colors buffer.
	glGenBuffers(1, &mesh.colors);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.colors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Colori) * meshData->vertCount, meshData->colors, type);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
	glEnableVertexAttribArray(2);

	// Alpha buffer.
	glGenBuffers(1, &mesh.alpha);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.alpha);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uint8_t) * meshData->vertCount, meshData->alpha, type);
	glVertexAttribPointer(3, 1, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
	glEnableVertexAttribArray(3);

	for (int i = 0; i < MESH_TYPE_COUNT; i++)
	{
		MeshIndexData& indexData = meshData->indices[i];

		if (indexData.count > 0)
		{
			MeshIndices& indices = mesh.indices[i];
			assert(indices.count == 0);

			// Index buffer.
			glGenBuffers(1, &indices.handle);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.handle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indexData.count, indexData.data, type);

			indices.count = indexData.count;
			assert(indices.count > 0);
		}
	}

	mesh.hasData = true;
	pool.Return(meshData);
}

static void FillMeshData(ObjectPool<MeshData2D>& pool, Mesh2D& mesh, MeshData2D* meshData, GLenum type, int32_t flags)
{
	glGenVertexArrays(1, &mesh.va);
	glBindVertexArray(mesh.va);

	int id = 0;

	glGenBuffers(1, &mesh.positions);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.positions);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 4, meshData->positions, type);
	glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(id++);

	if (!HasFlag(flags, MESH_NO_UVS))
	{
		glGenBuffers(1, &mesh.uvs);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.uvs);
		glBufferData(GL_ARRAY_BUFFER, sizeof(u16vec2) * 4, meshData->uvs, type);
		glVertexAttribPointer(id, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL); 
		glEnableVertexAttribArray(id++);
	}

	if (!HasFlag(flags, MESH_NO_COLORS))
	{
		glGenBuffers(1, &mesh.colors);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.colors);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Colori) * 4, meshData->colors, type);
		glVertexAttribPointer(id, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
		glEnableVertexAttribArray(id);
	}

	glGenBuffers(1, &mesh.indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, meshData->indices, type);

	mesh.flags = flags;
	pool.Return(meshData);
}

static inline void DrawMesh(Mesh mesh, int index)
{
	MeshIndices indices = mesh.indices[index];
	assert(indices.count > 0);
	glBindVertexArray(mesh.va);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.handle);
	glDrawElements(GL_TRIANGLES, indices.count, GL_UNSIGNED_SHORT, 0);
}

static inline void DrawMesh(Mesh mesh, Shader* shader, vec3 pos, int index)
{
	mat4 model = translate(mat4(1.0f), pos);
	SetUniform(shader->model, model);
	DrawMesh(mesh, index);
}

static inline void DrawMesh(Mesh2D mesh)
{
	glBindVertexArray(mesh.va);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

static inline void DrawMesh(Mesh2D mesh, Shader* shader, vec3 pos)
{
	mat4 model = translate(mat4(1.0f), pos);
	SetUniform(shader->model, model);
	DrawMesh(mesh);
}

static void DestroyMesh(Mesh& mesh)
{
	if (!mesh.hasData)
		return;

	for (int i = 0; i < MESH_TYPE_COUNT; i++)
	{
		MeshIndices& indices = mesh.indices[i];

		if (indices.count > 0)
		{
			glDeleteBuffers(1, &indices.handle);
			indices.count = 0;
		}
	}

	glDeleteBuffers(1, &mesh.positions);
	glDeleteBuffers(1, &mesh.uvs);
	glDeleteBuffers(1, &mesh.colors);
	glDeleteVertexArrays(1, &mesh.va);

	mesh.hasData = false;
}

static void DestroyMesh(Mesh2D& mesh)
{
	glDeleteBuffers(1, &mesh.positions);

	if (!HasFlag(mesh.flags, MESH_NO_UVS))
		glDeleteBuffers(1, &mesh.uvs);

	if (!HasFlag(mesh.flags, MESH_NO_COLORS))
		glDeleteBuffers(1, &mesh.colors);

	glDeleteBuffers(1, &mesh.indices);
	glDeleteVertexArrays(1, &mesh.va);
}
