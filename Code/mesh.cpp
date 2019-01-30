//
// Jason Bricco
//

static inline void SetMeshFlags(Mesh& mesh, int32_t flags)
{
	mesh.flags |= flags;
}

static inline bool MeshHasFlag(Mesh& mesh, int32_t flag)
{
	return mesh.flags & flag;
}

static MeshData* CreateMeshData(int vertices, int indices)
{
	MeshData* data = PushStruct(MeshData);
	data->positions = PushArray(vertices, vec3);
	data->texCoords = PushArray(vertices, vec3);
	data->colors = PushArray(vertices, Colori);
	data->indices = PushArray(indices, int);
	data->vertexMax = vertices;
	data->indexMax = indices;
	return data;
}

static void CreateMeshDataPool(MeshDataPool& pool, int capacity, int vertices, int indices)
{
	pool.capacity = capacity;
	pool.count = capacity;
    pool.data = PushArray(capacity, MeshData*);

    for (int i = 0; i < capacity; i++)
        pool.data[i] = CreateMeshData(vertices, indices);
}

static MeshData* CreateTempMeshData(int vertices, int indices)
{
	MeshData* data = PushTempStruct(MeshData);
	data->positions = PushTempArray(vertices, vec3);
	data->texCoords = PushTempArray(vertices, vec3);
	data->colors = PushTempArray(vertices, Colori);
	data->indices = PushTempArray(indices, int);;
	data->vertexMax = vertices;
	data->indexMax = indices;
	return data;
}

static inline bool MeshDataInBounds(MeshData* data, int inc, int count, int max)
{
	if (count + inc > max)
	{
		data->valid = false;
		return false;
	}

	return true;
}

static inline void SetIndices(MeshData* data)
{
	int offset = data->vertexCount;
	int count = data->indexCount;

	assert(MeshDataInBounds(data, 6, count, data->indexMax));

	data->indices[count] = offset + 2;
	data->indices[count + 1] = offset + 1;
	data->indices[count + 2] = offset;

	data->indices[count + 3] = offset + 3;
	data->indices[count + 4] = offset + 2;
	data->indices[count + 5] = offset;

	data->indexCount += 6;
}

static inline void SetUVs(MeshData* data, float w)
{
	int count = data->vertexCount;
	data->texCoords[count] = vec3(0.0f, 1.0f, w);
    data->texCoords[count + 1] = vec3(0.0f, 0.0f, w);
    data->texCoords[count + 2] = vec3(1.0f, 0.0f, w);
    data->texCoords[count + 3] = vec3(1.0f, 1.0f, w);
}

static void FillMeshData(Mesh& mesh, MeshData* meshData, GLenum type)
{
	glGenVertexArrays(1, &mesh.va);
	glBindVertexArray(mesh.va);

	int id = 0;

	// Vertex position buffer.
	glGenBuffers(1, &mesh.positions);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.positions);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * meshData->vertexCount, meshData->positions, type);

	glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(id++);

	if (!MeshHasFlag(mesh, MESH_NO_UVS))
	{
		// Vertex texture coordinates buffer.
		glGenBuffers(1, &mesh.texCoords);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.texCoords);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * meshData->vertexCount, meshData->texCoords, type);

		glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
		glEnableVertexAttribArray(id++);
	}

	if (!MeshHasFlag(mesh, MESH_NO_COLORS))
	{
		// Colors buffer.
		glGenBuffers(1, &mesh.colors);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.colors);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Colori) * meshData->vertexCount, meshData->colors, type);

		glVertexAttribPointer(id, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
		glEnableVertexAttribArray(id);
	}

	// Index buffer.
	glGenBuffers(1, &mesh.indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * meshData->indexCount, meshData->indices, type);

	mesh.indexCount = meshData->indexCount;
}

static inline void DrawMesh(Mesh& mesh)
{
	glBindVertexArray(mesh.va);
	glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
}

static inline void DrawMesh(Mesh& mesh, Shader* shader, vec3 pos)
{
	mat4 model = translate(mat4(1.0f), pos);
	SetUniform(shader->model, model);
	DrawMesh(mesh);
}

static void DestroyMesh(Mesh& mesh)
{
	if (mesh.indexCount > 0)
	{
		glDeleteBuffers(1, &mesh.positions);

		if (!MeshHasFlag(mesh, MESH_NO_UVS))
			glDeleteBuffers(1, &mesh.texCoords);

		if (!MeshHasFlag(mesh, MESH_NO_COLORS))
			glDeleteBuffers(1, &mesh.colors);

		glDeleteBuffers(1, &mesh.indices);
		glDeleteBuffers(1, &mesh.va);
		mesh.indexCount = 0;
	}
}
