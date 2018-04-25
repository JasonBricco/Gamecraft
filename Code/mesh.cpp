// Voxel Engine
// Jason Bricco

static void InitializeMesh(Mesh* mesh)
{
	glGenVertexArrays(1, &mesh->va);
	glBindVertexArray(mesh->va);

	// Vertex position attribute buffer.
	glGenBuffers(1, &mesh->vb);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, MESH_PARAMS * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(0);

	// Texture coordinates (UVs).
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, MESH_PARAMS * sizeof(GLfloat),
		(GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Vertex color attribute buffer.
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, MESH_PARAMS * sizeof(GLfloat),
		(GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// Index buffer.
	glGenBuffers(1, &mesh->ib);

	mesh->vertices.reserve(131072);
	mesh->indices.reserve(262144);
}

static void DestroyMesh(Mesh* mesh)
{
	glDeleteBuffers(1, &mesh->vb);
	glDeleteBuffers(1, &mesh->ib);
	glDeleteVertexArrays(1, &mesh->va);

	mesh->vertices.clear();
	mesh->indices.clear();
}

inline void SetMeshVertex(Mesh* mesh, float x, float y, float z, float u, float v, float tex,
	float r, float g, float b, float a)
{
	mesh->vertices.push_back(x);
	mesh->vertices.push_back(y);
	mesh->vertices.push_back(z);

	mesh->vertices.push_back(u);
	mesh->vertices.push_back(v);
	mesh->vertices.push_back(tex);

	mesh->vertices.push_back(r);
	mesh->vertices.push_back(g);
	mesh->vertices.push_back(b);
	mesh->vertices.push_back(a);
}

inline void SetMeshIndices(Mesh* mesh)
{
	int offset = (int)mesh->vertices.size() / MESH_PARAMS;

	mesh->indices.push_back(offset + 2);
	mesh->indices.push_back(offset + 1);
	mesh->indices.push_back(offset);

	mesh->indices.push_back(offset + 3);
	mesh->indices.push_back(offset + 2);
	mesh->indices.push_back(offset);
}

static void FillMeshData(Mesh* mesh)
{
	int vertexCount = (int)mesh->vertices.size();
	int indexCount = (int)mesh->indices.size();

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexCount, mesh->vertices.data(),
		GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * indexCount, mesh->indices.data(),
		GL_DYNAMIC_DRAW);
}

static void DrawMesh(Mesh* mesh)
{
	glBindVertexArray(mesh->va);
	glDrawElements(GL_TRIANGLES, (int)mesh->indices.size(), GL_UNSIGNED_INT, 0);
}
