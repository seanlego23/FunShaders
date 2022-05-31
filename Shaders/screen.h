#pragma once

#include <glad/glad.h>

class screen {

	GLuint _vao;
	GLuint _vbo;
	int _width, _height;

	float _time = 0.0f;

public:

	screen();
	~screen();

	void resetTime();

	void draw_screen(class shader* s, const struct shader_inputs* si);
};

