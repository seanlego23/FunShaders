#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

struct shader_inputs {
	float elapsedTime = 0.0f;
	float zoom = 1.0f;
	float zoomRaw = 0.0f;

	shader_inputs() { }

	shader_inputs(float dt, float z, float zr) : elapsedTime(dt), zoom(z), zoomRaw(zr) { }

	virtual void send_data(GLuint program) const {
		glUniform1f(glGetUniformLocation(program, "elapsedTime"), elapsedTime);
		glUniform1f(glGetUniformLocation(program, "zoom"), zoom);
		glUniform1f(glGetUniformLocation(program, "zoomRaw"), zoomRaw);
	}
};