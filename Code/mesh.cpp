//
// Jason Bricco
//

static inline bool MeshHasFlag(Mesh* mesh, int32_t flag)
{
	return mesh->flags & flag;
}

static inline int CalculateMeshDataSize(int vertices, int indices)
{
	return (vertices * sizeof(vec3) * 2) + (vertices * sizeof(Colori)) + (indices * sizeof(int));
}

static Mesh* CreateMesh(int vertices, int indices, int32_t flags)
{
	Mesh* mesh = AllocStruct(Mesh);
	mesh->flags = flags;
	int sizeInBytes = CalculateMeshDataSize(vertices, indices);
	mesh->data = AllocRaw(sizeInBytes);
	mesh->vertCount = 0;
	mesh->vertMax = vertices;
	mesh->indexCount = 0;
	mesh->indexMax = indices;
	mesh->positionData = (vec3*)mesh->data;
	mesh->uvData = mesh->positionData + vertices;
	mesh->colorData = (Colori*)(mesh->uvData + vertices);
	mesh->indexData = (int*)(mesh->colorData + vertices);

	return mesh;
}

static inline void CheckMeshBounds(Mesh* mesh, int count, int inc, int max)
{
	if (count + inc > max)
	{
		mesh->vertMax *= 2;
		mesh->indexMax *= 2;
		mesh->data = ReallocRaw(mesh->data, CalculateMeshDataSize(mesh->vertMax, mesh->indexMax));
	}
}

static inline void SetIndices(Mesh* mesh)
{
	CheckMeshBounds(mesh, mesh->indexCount, 6, mesh->indexMax);

	int offset = mesh->vertCount;
	int count = mesh->indexCount;

	mesh->indexData[count] = offset + 2;
	mesh->indexData[count + 1] = offset + 1;
	mesh->indexData[count + 2] = offset;

	mesh->indexData[count + 3] = offset + 3;
	mesh->indexData[count + 4] = offset + 2;
	mesh->indexData[count + 5] = offset;

	mesh->indexCount += 6;
}

static inline void SetUVs(Mesh* mesh, float w)
{
	int count = mesh->vertCount;
	mesh->uvData[count] = vec3(0.0f, 1.0f, w);
    mesh->uvData[count + 1] = vec3(0.0f, 0.0f, w);
    mesh->uvData[count + 2] = vec3(1.0f, 0.0f, w);
    mesh->uvData[count + 3] = vec3(1.0f, 1.0f, w);
}

static void FillMeshData(Mesh* mesh, GLenum type)
{
	BEGIN_TIMED_BLOCK(FILL_MESH);

	glGenVertexArrays(1, &mesh->va);
	glBindVertexArray(mesh->va);

	int id = 0;

	// Vertex position buffer.
	glGenBuffers(1, &mesh->positions);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->positions);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * mesh->vertCount, mesh->positionData, type);

	glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(id++);

	if (!MeshHasFlag(mesh, MESH_NO_UVS))
	{
		// Vertex texture coordinates buffer.
		glGenBuffers(1, &mesh->uvs);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->uvs);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * mesh->vertCount, mesh->uvData, type);

		glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
		glEnableVertexAttribArray(id++);
	}

	if (!MeshHasFlag(mesh, MESH_NO_COLORS))
	{
		// Colors buffer.
		glGenBuffers(1, &mesh->colors);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->colors);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Colori) * mesh->vertCount, mesh->colorData, type);

		glVertexAttribPointer(id, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
		glEnableVertexAttribArray(id);
	}

	// Index buffer.
	glGenBuffers(1, &mesh->indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * mesh->indexCount, mesh->indexData, type);

	Free(mesh->data);
	mesh->data = nullptr;

	END_TIMED_BLOCK(FILL_MESH);
}

static inline void DrawMesh(Mesh* mesh)
{
	glBindVertexArray(mesh->va);
	glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
}

static inline void DrawMesh(Mesh* mesh, Shader* shader, vec3 pos)
{
	mat4 model = translate(mat4(1.0f), pos);
	SetUniform(shader->model, model);
	DrawMesh(mesh);
}

static void DestroyMesh(Mesh* mesh)
{
	if (mesh != nullptr)
	{
		assert(mesh->indexCount > 0);
		assert(mesh->data == nullptr);

		glDeleteBuffers(1, &mesh->positions);

		if (!MeshHasFlag(mesh, MESH_NO_UVS))
			glDeleteBuffers(1, &mesh->uvs);

		if (!MeshHasFlag(mesh, MESH_NO_COLORS))
			glDeleteBuffers(1, &mesh->colors);

		glDeleteBuffers(1, &mesh->indices);
		glDeleteVertexArrays(1, &mesh->va);
		Free(mesh);
	}
}
