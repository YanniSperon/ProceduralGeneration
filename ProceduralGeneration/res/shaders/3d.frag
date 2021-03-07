#version 460

in vec3 normal;
in vec2 texCoord;
in vec3 fragWorldPos;

layout (binding = 0) uniform sampler2D image;

out vec4 color;

vec3 getNormal() {
	vec3 tempX = dFdx(fragWorldPos);
	vec3 tempY = dFdy(fragWorldPos);

	vec3 calcNormal = normalize(cross(tempX, tempY));

	return calcNormal;
}

void main() {
	//color = texture(image, texCoord);
	vec3 newNormal = getNormal();
	float grayscaleColor = (newNormal.r + newNormal.g + newNormal.b)/3.0;
	float weightedColor = 0.5 + (0.5 * grayscaleColor);
	color = vec4(weightedColor, weightedColor, weightedColor, 1.0);
}