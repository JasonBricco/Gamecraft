//
// Jason Bricco
//

static Mesh* CreateMesh(int vertMax = 131072, int indexMax = 65536)
{
	Mesh* mesh = new Mesh();

	mesh->vertMax = vertMax;
	mesh->indexMax = indexMax;

	mesh->vertices = (float*)malloc(mesh->vertMax * sizeof(float));
	mesh->indices = (int*)malloc(mesh->indexMax * sizeof(int));

	return mesh;
}

template <typename T>
static inline T* ExpandIfNeeded(T* data, int adding, int count, int& max)
{
	if (count + adding > max)
	{
		data = (T*)realloc(data, max * 2 * sizeof(T));
		max *= 2;
	}

	return data;
}

static inline void SetMeshVertex(Mesh* mesh, float x, float y, float z, float u, float v, float tex, Color c)
{
	mesh->vertices = ExpandIfNeeded(mesh->vertices, 10, mesh->vertCount, mesh->vertMax);
	int count = mesh->vertCount;

	mesh->vertices[count++] = x;
	mesh->vertices[count++] = y;
	mesh->vertices[count++] = z;

	mesh->vertices[count++] = u;
	mesh->vertices[count++] = v;
	mesh->vertices[count++] = tex;

	mesh->vertices[count++] = c.r;
	mesh->vertices[count++] = c.g;
	mesh->vertices[count++] = c.b;
	mesh->vertices[count++] = c.a;

	mesh->vertCount = count;
}

static inline void SetMeshVertex(Mesh* mesh, float x, float y, float u, float v)
{
	mesh->vertices = ExpandIfNeeded(mesh->vertices, 4, mesh->vertCount, mesh->vertMax);
	int count = mesh->vertCount;

	mesh->vertices[count++] = x;
	mesh->vertices[count++] = y;
	mesh->vertices[count++] = u;
	mesh->vertices[count++] = v;
	
	mesh->vertCount = count;
}

static inline void SetMeshVertex(Mesh* mesh, float x, float y)
{
	mesh->vertices = ExpandIfNeeded(mesh->vertices, 2, mesh->vertCount, mesh->vertMax);
	int count = mesh->vertCount;

	mesh->vertices[count++] = x;
	mesh->vertices[count++] = y;

	mesh->vertCount = count;
}

static inline void SetMeshIndices(Mesh* mesh, int params)
{
	mesh->indices = ExpandIfNeeded(mesh->indices, 6, mesh->indexCount, mesh->indexMax);
	int offset = mesh->vertCount / params;
	int count = mesh->indexCount;

	mesh->indices[count++] = offset + 2;
	mesh->indices[count++] = offset + 1;
	mesh->indices[count++] = offset;

	mesh->indices[count++] = offset + 3;
	mesh->indices[count++] = offset + 2;
	mesh->indices[count++] = offset;

	mesh->indexCount = count;
}

static inline void FillMeshData(Mesh* mesh, GLenum type, VertexSpec spec)
{
	glGenVertexArrays(1, &mesh->va);
	glBindVertexArray(mesh->va);

	// Vertex buffer.
	glGenBuffers(1, &mesh->vb);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mesh->vertCount, mesh->vertices, type);

	// Index buffer.
	glGenBuffers(1, &mesh->ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * mesh->indexCount, mesh->indices, type);

	int params = spec.numPositions + spec.numUvs + spec.numColors;
	int id = 0, offset = 0;

	if (spec.position)
	{
		glVertexAttribPointer(id, spec.numPositions, GL_FLOAT, GL_FALSE, params * sizeof(GLfloat), NULL);
		glEnableVertexAttribArray(id++);
		offset += spec.numPositions;
	}

	if (spec.texture)
	{
		glVertexAttribPointer(id, spec.numUvs, GL_FLOAT, GL_FALSE, params * sizeof(GLfloat), (GLvoid*)(offset * sizeof(GLfloat))); 
		glEnableVertexAttribArray(id++);
		offset += spec.numUvs;
	}

	if (spec.color)
	{
		glVertexAttribPointer(id, 4, GL_FLOAT, GL_FALSE, params * sizeof(GLfloat), (GLvoid*)(offset * sizeof(GLfloat)));
		glEnableVertexAttribArray(id);
	}

	free(mesh->vertices);
	free(mesh->indices);

	mesh->vertices = nullptr;
	mesh->indices = nullptr;
}

static void DestroyMesh(Mesh* mesh)
{
	if (mesh == nullptr) return;

	if (mesh->vertCount > 0)
	{
		glDeleteBuffers(1, &mesh->vb);
		glDeleteBuffers(1, &mesh->ib);
		glDeleteVertexArrays(1, &mesh->va);
	}

	if (mesh->vertices != nullptr)
	{
		free(mesh->vertices);
		free(mesh->indices);
	}

	delete mesh;
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
