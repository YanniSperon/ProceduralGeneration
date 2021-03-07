#version 460

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transformation;

out vec3 normal;
out vec2 texCoord;
out vec3 fragWorldPos;

void main()
{
	texCoord = vertexTexCoord;
	normal = mat3(transpose(inverse(transformation))) * vertexNormal;
	fragWorldPos = vec3(transformation * vec4(vertexPosition, 1.0));

	gl_Position = projection * view * vec4(fragWorldPos, 1.0);
}