#pragma once

#include<glm/glm.hpp>

struct shader_inputs {
	glm::vec2 resolution;
	glm::vec2 translation;
	glm::vec2 scale;
	glm::vec2 offset;
	glm::vec2 cursorPos;
	float elapsedTime;
	float zoom;
	float zoomSum;
};