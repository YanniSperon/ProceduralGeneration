#version 460

in vec2 texCoord;

layout (binding = 0) uniform sampler2D image;

out vec4 color;

void main() {
	color = texture(image, texCoord);
	//color = vec4(0.0, 0.0, 0.0, 1.0);
}
