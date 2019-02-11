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

static inline int CalculateMeshDataSize(int vertices, int indices)
{
	return (vertices * sizeof(vec3) * 2) + (vertices * sizeof(Colori)) + (indices * sizeof(int));
}

static MeshData* CreateMeshData(int vertices, int indices)
{
	MeshData* meshData = (MeshData*)malloc(sizeof(MeshData));
	meshData->data = malloc(CalculateMeshDataSize(vertices, indices));
	meshData->vertCount = 0;
	meshData->vertMax = vertices;
	meshData->indexCount = 0;
	meshData->indexMax = indices;
	meshData->positions = (vec3*)meshData->data;
	meshData->texCoords = meshData->positions + vertices;
	meshData->colors = (Colori*)(meshData->texCoords + vertices);
	meshData->indices = (int*)(meshData->colors + vertices);
	return meshData;
}

static void DestroyMeshData(MeshData* meshData)
{
	free(meshData->data);
}

static inline void CheckMeshBounds(MeshData* meshData, int count, int inc, int max)
{
	if (count + inc > max)
	{
		meshData->vertMax *= 2;
		meshData->indexMax *= 2;
		meshData->data = realloc(meshData->data, CalculateMeshDataSize(meshData->vertMax, meshData->indexMax));
	}
}

static inline void SetIndices(MeshData* data)
{
	CheckMeshBounds(data, data->indexCount, 6, data->indexMax);

	int offset = data->vertCount;
	int count = data->indexCount;

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
	int count = data->vertCount;
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * meshData->vertCount, meshData->positions, type);

	glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(id++);

	if (!MeshHasFlag(mesh, MESH_NO_UVS))
	{
		// Vertex texture coordinates buffer.
		glGenBuffers(1, &mesh.texCoords);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.texCoords);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * meshData->vertCount, meshData->texCoords, type);

		glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
		glEnableVertexAttribArray(id++);
	}

	if (!MeshHasFlag(mesh, MESH_NO_COLORS))
	{
		// Colors buffer.
		glGenBuffers(1, &mesh.colors);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.colors);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Colori) * meshData->vertCount, meshData->colors, type);

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
