#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "camera.h"

class screen {

	GLuint _vao;
	GLuint _vbo;

	float _time = 0.0f;

	glm::vec2 _resolution;
	glm::vec2 _cursorPos;

public:

	Camera camera;

	screen();
	~screen();

	void resetTime();

	glm::vec2 getResolution() const;

	void setResolution(glm::vec2 res);

	glm::vec2 getCursorPos() const;

	void setCursorPos(glm::vec2 curpos);

	void draw_screen(class shader_object* obj);
};

