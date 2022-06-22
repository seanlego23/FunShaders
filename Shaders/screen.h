#pragma once

#include <glad/glad.h>

#include "camera.h"

class screen {

	GLuint _vao;
	GLuint _vbo;

	float _time = 0.0f;

public:

	Camera camera;

	screen();
	~screen();

	void resetTime();

	void draw_screen(const class shader_object* obj);
};

