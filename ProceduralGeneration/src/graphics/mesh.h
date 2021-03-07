#pragma once

#include "vertex.h"

#include <string>
#include <glew.h>

class Mesh3D {
private:
	Vertex3D* m_Vertices;
	GLuint m_NumVertices;
	GLuint* m_Indices;
	GLuint m_NumIndices;
	GLuint m_VRAMHandleVAO;
	GLuint m_VRAMHandleVBO;
	GLuint m_VRAMHandleIBO;

	std::string m_Path;
public:
	Mesh3D(const std::string& path);
	~Mesh3D();

	static void Unbind();

	void Bind();
	void Draw();

	GLsizeiptr VertexBufferSize();
	GLsizeiptr IndexBufferSize();

	void LoadToRAM(const std::string& path);
	void UnloadFromRAM();

	void LoadToVRAM();
	void UnloadFromVRAM();

	Vertex3D* GetVertices();
	unsigned int GetNumberOfVertices();
	GLuint* GetIndices();
	unsigned int GetNumberOfIndices();
};