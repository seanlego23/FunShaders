#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "camera.h"

class screen {

	GLuint _vao;
	GLuint _vbo;

	float _time = 0.0f;

	glm::vec2 _resolution;

public:

	Camera camera;

	screen();
	~screen();

	void resetTime();

	glm::vec2 getResolution();

	void setResolution(glm::vec2 res);

	void draw_screen(const class shader_object* obj);
};

