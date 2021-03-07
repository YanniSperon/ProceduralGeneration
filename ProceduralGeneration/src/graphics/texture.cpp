#include "texture.h"
#include "utilities/console.h"

#include "vendor/stb/stb_image.h"
//#include "vendor/opensimplexnoise/OpenSimplexNoise.h"

#include <noise/noise.h>

Texture::Texture(unsigned int width, unsigned int height)
	: m_ID(0), m_Path(std::to_string(width) + std::to_string(height))
{
	GenerateTexture(width, height);
}

Texture::Texture(const std::string& name)
	: m_ID(0), m_Path(name)
{
	LoadTexture(name);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_ID);
	Console::Warning("Deleted texture");
}

void Texture::LoadTexture(const std::string& name)
{
	glGenTextures(1, &m_ID);
	stbi_set_flip_vertically_on_load(1);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(name.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		GLenum format = GL_RED;
		if (nrChannels == 1) {
			format = GL_RED;
		}
		else if (nrChannels == 3) {
			format = GL_RGB;
		}
		else if (nrChannels == 4) {
			format = GL_RGBA;
		}
		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);

		stbi_image_free(data);
	}
	else
	{
		stbi_image_free(data);
		Console::Error("Failed to load texture \"%s\"", name.c_str());
	}
}

void Texture::GenerateTexture(unsigned int width, unsigned int height)
{
	Console::Info("Generating texture");
	glGenTextures(1, &m_ID);

	unsigned int numberOfChannels = 4;

	unsigned int totalSize = width * height * 4;
	unsigned int totalByteSize = totalSize * 8;

	unsigned char* data = (unsigned char*) malloc(totalSize);
	
	noise::module::Perlin perlinModule;
	perlinModule.SetSeed(1000);
	perlinModule.SetFrequency(10);
	//OpenSimplexNoise::Noise noise;

	if (data) {
		Console::Info("    Allocated data");
		if (totalSize >= 4) {
			Console::Info("    Began setting values");
			int count = 0;
			for (unsigned int i = 0; i < totalSize; i += 4) {
				double xCoord = (((i % (width * 4))/4)/(double)width);
				double yCoord = (((i / (width * 4)))/(double)height);
				double calculatedNoiseValue = perlinModule.GetValue(xCoord, yCoord, 0.0f);
				//double calculatedNoiseValue = noise.eval(xCoord, yCoord);
				unsigned char noiseValue = ((calculatedNoiseValue + 1.0) / 2.0) * 255;
				data[i] = noiseValue;
				data[i + 1] = noiseValue;
				data[i + 2] = noiseValue;
				data[i + 3] = noiseValue;
				count++;
			}
			Console::Info("    Iterated %i times", count);

			glBindTexture(GL_TEXTURE_2D, m_ID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
		}
		free(data);
	}
}

void Texture::Bind(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

GLuint Texture::GetID()
{
	return m_ID;
}

std::string Texture::GetPath()
{
	return m_Path;
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
