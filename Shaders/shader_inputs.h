#pragma once

#include<glad/glad.h>
#include<glm/glm.hpp>

struct shader_inputs {
	float elapsedTime;
	float zoom;
	float zoomRaw;

	void send_uniforms(GLuint program) const {
		glUniform1f(glGetUniformLocation(program, "elapsedTime"), elapsedTime);
		glUniform1f(glGetUniformLocation(program, "zoom"), zoom);
		glUniform1f(glGetUniformLocation(program, "zoomRaw"), zoomRaw);
	}
};