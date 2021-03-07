#pragma once

#include "vendor/glm/glm.hpp"

struct Vertex3D {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};