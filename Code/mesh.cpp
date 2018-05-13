// Voxel Engine
// Jason Bricco

static Mesh* CreateMesh()
{
	Mesh* mesh = Calloc(Mesh, sizeof(Mesh), "Mesh");

	mesh->vertMax = 131072;
	mesh->indexMax = 65536;

	mesh->vertices = Malloc(float, mesh->vertMax * sizeof(float), "Vertices");
	mesh->indices = Malloc(int, mesh->indexMax * sizeof(int), "Indices");

	return mesh;
}

static void DestroyMesh(Mesh* mesh)
{
	if (mesh == NULL) return;

	if (mesh->vertCount > 0)
	{
		glDeleteBuffers(1, &mesh->vb);
		glDeleteBuffers(1, &mesh->ib);
		glDeleteVertexArrays(1, &mesh->va);
	}

	if (mesh->vertices != NULL)
	{
		Free(mesh->vertices, "Vertices");
		Free(mesh->indices, "Indices");
	}

	Free(mesh, "Mesh");
}

inline void SetMeshVertex(Mesh* mesh, float x, float y, float z, float u, float v, float tex,
	float r, float g, float b, float a)
{
	if (mesh->vertCount + MESH_PARAMS > mesh->vertMax)
	{
		mesh->vertices = (float*)realloc(mesh->vertices, mesh->vertMax * 2 * sizeof(float));
		mesh->vertMax *= 2;
	}

	int count = mesh->vertCount;

	mesh->vertices[count++] = x;
	mesh->vertices[count++] = y;
	mesh->vertices[count++] = z;

	mesh->vertices[count++] = u;
	mesh->vertices[count++] = v;
	mesh->vertices[count++] = tex;

	mesh->vertices[count++] = r;
	mesh->vertices[count++] = g;
	mesh->vertices[count++] = b;
	mesh->vertices[count++] = a;

	mesh->vertCount = count;
}

inline void SetMeshIndices(Mesh* mesh)
{
	if (mesh->indexCount + 6 > mesh->indexMax)
	{
		mesh->indices = (int*)realloc(mesh->indices, mesh->indexMax * 2 * sizeof(float));
		mesh->indexMax *= 2;
	}

	int offset = mesh->vertCount / MESH_PARAMS;
	int count = mesh->indexCount;

	mesh->indices[count++] = offset + 2;
	mesh->indices[count++] = offset + 1;
	mesh->indices[count++] = offset;

	mesh->indices[count++] = offset + 3;
	mesh->indices[count++] = offset + 2;
	mesh->indices[count++] = offset;

	mesh->indexCount = count;
}

static void FillMeshData(Mesh* mesh)
{
	BEGIN_TIMED_BLOCK(FILL_MESH);

	if (mesh->vertCount > 0)
	{
		glGenVertexArrays(1, &mesh->va);
		glBindVertexArray(mesh->va);

		// Vertex position attribute buffer.
		glGenBuffers(1, &mesh->vb);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vb);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mesh->vertCount, mesh->vertices, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, MESH_PARAMS * sizeof(GLfloat), NULL);
		glEnableVertexAttribArray(0);

		// Texture coordinates (UVs).
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, MESH_PARAMS * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		// Vertex color attribute buffer.
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, MESH_PARAMS * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);

		// Index buffer.
		glGenBuffers(1, &mesh->ib);
 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ib);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * mesh->indexCount, mesh->indices, GL_DYNAMIC_DRAW);
	}

	Free(mesh->vertices, "Vertices");
	Free(mesh->indices, "Indices");

	mesh->vertices = NULL;
	mesh->indices = NULL;

	END_TIMED_BLOCK(FILL_MESH);
}

static void DrawMesh(Mesh* mesh)
{
	glBindVertexArray(mesh->va);
	glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
}
