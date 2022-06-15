#pragma once

#include <glm/glm.hpp>

struct Camera {
	glm::vec3 loc;
	glm::vec3 lookAt;
	glm::vec3 fov;
};