#include "screen.h"
#include "shader.h"
#include "shader_inputs.h"

static GLfloat screen_coords[] = {
	-1.0f, -1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,
};

screen::screen() {

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

void screen::draw_screen(shader* s, const shader_inputs* si) {
	_time += si->elapsedTime;

	s->use();

	GLuint prog = s->getProgram();

	glUniform2f(glGetUniformLocation(prog, "resolution"), si->resolution.x, si->resolution.y);
	glUniform2f(glGetUniformLocation(prog, "translation"), si->translation.x, si->translation.y);
	glUniform2f(glGetUniformLocation(prog, "scale"), si->scale.x, si->scale.y);
	glUniform2f(glGetUniformLocation(prog, "offset"), si->offset.x, si->offset.y);
	glUniform1f(glGetUniformLocation(prog, "time"), _time);
	glUniform1f(glGetUniformLocation(prog, "elapsedTime"), si->elapsedTime);
	glUniform1f(glGetUniformLocation(prog, "zoom"), si->zoom);

	glBindVertexArray(_vao);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
