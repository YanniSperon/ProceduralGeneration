#pragma once

#include <glew.h>
#include <string>

class Texture {
private:
	GLuint m_ID;

	std::string m_Path;

public:
	Texture(unsigned int width, unsigned int height);
	Texture(const std::string& name);
	~Texture();

	void LoadTexture(const std::string& name);
	void GenerateTexture(unsigned int width, unsigned int height);

	void Bind(int slot);

	GLuint GetID();

	std::string GetPath();

	static void Unbind();
};
