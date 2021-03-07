#include "mesh.h"
#include "utilities/console.h"
#include "vertex.h"

#include <vector>
#include <sstream>
#include <fstream>

Mesh3D::Mesh3D(const std::string& path)
	: m_Path(path)
{
	LoadToRAM(path);
}

Mesh3D::~Mesh3D()
{
	UnloadFromRAM();
	UnloadFromVRAM();
}

void Mesh3D::Unbind()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh3D::Bind()
{
	glBindVertexArray(m_VRAMHandleVAO);
}

void Mesh3D::Draw()
{
	glDrawElements(GL_TRIANGLES, m_NumIndices, GL_UNSIGNED_INT, 0);
}

GLsizeiptr Mesh3D::VertexBufferSize()
{
	return m_NumVertices * sizeof(Vertex3D);
}

GLsizeiptr Mesh3D::IndexBufferSize()
{
	return m_NumIndices * sizeof(GLuint);
}

void Mesh3D::LoadToRAM(const std::string& path)
{
	m_Path = path;
	std::vector<Vertex3D> tempVertices;
	std::vector<GLuint> tempIndices;
	std::vector<glm::vec2> tempTex;
	std::vector<glm::vec3> tempNormals;

	std::ifstream file(path);

	if (!file.is_open()) {
		Console::Error("Invalid file - does not exist! \"%s\"", path.c_str());
	}

	while (!file.eof())
	{
		char line[128];
		file.getline(line, 128);

		std::stringstream s;
		s << line;

		char junk;
		char slash = '/';

		if (line[0] == 'v' && line[1] == ' ')
		{
			glm::vec3 v;
			s >> junk >> v.x >> v.y >> v.z;
			Vertex3D vert;
			vert.position = v;
			tempVertices.push_back(vert);
		}
		else if (line[0] == 'v' && line[1] == 'n') {
			glm::vec3 v;
			s >> junk >> junk >> v.x >> v.y >> v.z;
			tempNormals.push_back(v);
		}
		else if (line[0] == 'f' && line[1] == ' ')
		{
			unsigned int f[3];
			unsigned int uv[3];
			unsigned int n[3];

			s >> junk >> f[0] >> slash >> uv[0] >> slash >> n[0] >> f[1] >> slash >> uv[1] >> slash >> n[1] >> f[2] >> slash >> uv[2] >> slash >> n[2];
			tempIndices.push_back(f[0] - 1);
			tempIndices.push_back(f[1] - 1);
			tempIndices.push_back(f[2] - 1);
			tempVertices.at(f[0] - 1).texCoord = glm::vec2(tempTex.at(uv[0] - 1).x, tempTex.at(uv[0] - 1).y);
			tempVertices.at(f[1] - 1).texCoord = glm::vec2(tempTex.at(uv[1] - 1).x, tempTex.at(uv[1] - 1).y);
			tempVertices.at(f[2] - 1).texCoord = glm::vec2(tempTex.at(uv[2] - 1).x, tempTex.at(uv[2] - 1).y);
			tempVertices.at(f[0] - 1).normal = tempNormals.at(n[0] - 1);
			tempVertices.at(f[1] - 1).normal = tempNormals.at(n[1] - 1);
			tempVertices.at(f[2] - 1).normal = tempNormals.at(n[2] - 1);
		}
		else if (line[0] == 'v' && line[1] == 't') {
			float one;
			float two;
			std::string name;
			s >> name;
			s >> one;
			s >> two;
			glm::vec2 uv;
			uv.x = one;
			uv.y = two;
			tempTex.push_back(uv);
		}
	}

	m_NumVertices = tempVertices.size();
	m_Vertices = new Vertex3D[m_NumVertices];
	memcpy(m_Vertices, tempVertices.data(), sizeof(Vertex3D) * m_NumVertices);

	m_NumIndices = tempIndices.size();
	m_Indices = new GLuint[m_NumIndices];
	memcpy(m_Indices, tempIndices.data(), sizeof(GLuint) * m_NumIndices);
}

void Mesh3D::UnloadFromRAM()
{
	delete[] m_Vertices;
	delete[] m_Indices;
	m_Vertices = nullptr;
	m_Indices = nullptr;
}

void Mesh3D::LoadToVRAM()
{
	UnloadFromVRAM();

	Unbind();

	glGenVertexArrays(1, &m_VRAMHandleVAO);
	glBindVertexArray(m_VRAMHandleVAO);

	glGenBuffers(1, &m_VRAMHandleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VRAMHandleVBO);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize(), m_Vertices, GL_STATIC_DRAW);
	Console::Info("Loaded %u data", VertexBufferSize());

	glGenBuffers(1, &m_VRAMHandleIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VRAMHandleIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize(), m_Indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

	glFinish();
}

void Mesh3D::UnloadFromVRAM()
{
	if (m_VRAMHandleVBO != 0) {
		glDeleteBuffers(1, &m_VRAMHandleVBO);
		glDeleteBuffers(1, &m_VRAMHandleIBO);
		glDeleteVertexArrays(1, &m_VRAMHandleVAO);

		m_VRAMHandleVBO = 0;
		m_VRAMHandleIBO = 0;
		m_VRAMHandleVAO = 0;
	}
}

Vertex3D* Mesh3D::GetVertices()
{
	return m_Vertices;
}

unsigned int Mesh3D::GetNumberOfVertices()
{
	return m_NumVertices;
}

GLuint* Mesh3D::GetIndices()
{
	return m_Indices;
}

unsigned int Mesh3D::GetNumberOfIndices()
{
	return m_NumIndices;
}

