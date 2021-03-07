#version 460

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 textureCoordinate;

uniform mat4 projection;
uniform mat4 transformation;

out vec2 texCoord;

void main()
{
	texCoord = textureCoordinate;
	gl_Position = projection * transformation * vec4(vertexPosition, 0.0, 1.0);
}
