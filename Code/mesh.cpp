//
// Jason Bricco
//

static MeshData* CreateMeshData(int verts, int indices)
{
	MeshData* data = PushStruct(MeshData);
	data->vertices = PushArray(verts, float);
	data->indices = PushArray(indices, int);
	data->vertMax = verts;
	data->indexMax = indices;
	return data;
}

static MeshData* CreateTempMeshData(int verts, int indices)
{
	MeshData* data = PushTempStruct(MeshData);
	data->vertices = PushTempArray(verts, float);
	data->indices = PushTempArray(indices, int);
	data->vertMax = verts;
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

static inline void SetMeshVertex(MeshData* data, float x, float y, float z, float u, float v)
{
	int count = data->vertCount;

	if (MeshDataInBounds(data, 5, count, data->vertMax))
	{
		data->vertices[count] = x;
		data->vertices[count + 1] = y;
		data->vertices[count + 2] = z;

		data->vertices[count + 3] = u;
		data->vertices[count + 4] = v;

		data->vertCount += 5;
	}
}

static inline void SetMeshVertex(MeshData* data, float x, float y, float z, float u, float v, float tex, Color c)
{
	int count = data->vertCount;

	if (MeshDataInBounds(data, 10, count, data->vertMax))
	{
		data->vertices[count] = x;
		data->vertices[count + 1] = y;
		data->vertices[count + 2] = z;

		data->vertices[count + 3] = u;
		data->vertices[count + 4] = v;
		data->vertices[count + 5] = tex;

		data->vertices[count + 6] = c.r;
		data->vertices[count + 7] = c.g;
		data->vertices[count + 8] = c.b;
		data->vertices[count + 9] = c.a;

		data->vertCount += 10;
	}
}

static inline void SetMeshVertex(MeshData* data, float x, float y, float u, float v)
{
	int count = data->vertCount;

	if (MeshDataInBounds(data, 4, count, data->vertMax))
	{
		data->vertices[count] = x;
		data->vertices[count + 1] = y;
		data->vertices[count + 2] = u;
		data->vertices[count + 3] = v;
		
		data->vertCount += 4;
	}
}

static inline void SetMeshVertex(MeshData* data, float x, float y)
{
	int count = data->vertCount;

	if (MeshDataInBounds(data, 2, count, data->vertMax))
	{
		data->vertices[count] = x;
		data->vertices[count + 1] = y;

		data->vertCount += 2;
	}
}

static inline void SetMeshIndices(MeshData* data, int params)
{
	int offset = data->vertCount / params;
	int count = data->indexCount;

	if (MeshDataInBounds(data, 6, count, data->indexMax))
	{
		data->indices[count] = offset + 2;
		data->indices[count + 1] = offset + 1;
		data->indices[count + 2] = offset;

		data->indices[count + 3] = offset + 3;
		data->indices[count + 4] = offset + 2;
		data->indices[count + 5] = offset;

		data->indexCount += 6;
	}
}

static inline void FillMeshData(Mesh& mesh, MeshData* meshData, GLenum type, VertexSpec spec)
{
	glGenVertexArrays(1, &mesh.va);
	glBindVertexArray(mesh.va);

	// Vertex buffer.
	glGenBuffers(1, &mesh.vb);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * meshData->vertCount, meshData->vertices, type);

	// Index buffer.
	glGenBuffers(1, &mesh.ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * meshData->indexCount, meshData->indices, type);

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

	mesh.vertCount = meshData->vertCount;
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
    if (mesh.vertCount > 0)
    {
        glDeleteBuffers(1, &mesh.vb);
        glDeleteBuffers(1, &mesh.ib);
        glDeleteVertexArrays(1, &mesh.va);

        mesh.vertCount = 0;
        mesh.indexCount = 0;
    }
}
