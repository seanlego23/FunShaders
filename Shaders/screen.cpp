#include "screen.h"
#include "shader.h"
#include "shader_inputs.h"
#include "shader_object.h"

static GLfloat screen_coords[] = {
	-1.0f, -1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,
};

screen::screen() : camera{} {

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);

	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_coords), screen_coords, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

screen::~screen() {
	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);
}

void screen::resetTime() {
	_time = 0.0f;
}

glm::vec2 screen::getResolution() {
	return _resolution;
}

void screen::setResolution(glm::vec2 res) {
	_resolution = res;
}

void screen::draw_screen(const shader_object* obj) {
	_time += obj->inputs.elapsedTime;

	obj->use();

	GLuint prog = obj->getProgram();
	obj->inputs.send_uniforms(prog);

	glUniform2fv(glGetUniformLocation(prog, "resolution"), 1, &_resolution.x);
	glUniform1f(glGetUniformLocation(prog, "time"), _time);
	glUniform3fv(glGetUniformLocation(prog, "camera.loc"), 1, &camera.loc.x);
	glUniform3fv(glGetUniformLocation(prog, "camera.lookAt"), 1, &camera.lookAt.x);
	glUniform1f(glGetUniformLocation(prog, "camera.fov"), camera.fov);

	glBindVertexArray(_vao);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
