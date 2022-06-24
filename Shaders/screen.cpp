#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

glm::vec2 screen::getResolution() const {
	return _resolution;
}

void screen::setResolution(glm::vec2 res) {
	_resolution = res;
}

glm::vec2 screen::getCursorPos() const {
	return _cursorPos;
}

void screen::setCursorPos(glm::vec2 curpos) {
	_cursorPos = curpos;
}

void screen::draw_screen(const shader_object* obj) {
	_time += obj->inputs.elapsedTime;

	glm::mat4 view = glm::lookAt(camera.loc, camera.lookAt, glm::vec3(0.0f, 0.0f, 1.0f));

	obj->use();

	GLuint prog = obj->getProgram();
	obj->inputs.send_uniforms(prog);

	glUniform2fv(glGetUniformLocation(prog, "resolution"), 1, glm::value_ptr(_resolution));
	glUniform2fv(glGetUniformLocation(prog, "cursorPos"), 1, glm::value_ptr(_cursorPos));
	glUniform3fv(glGetUniformLocation(prog, "camera.loc"), 1, glm::value_ptr(camera.loc));
	glUniform3fv(glGetUniformLocation(prog, "camera.lookAt"), 1, glm::value_ptr(camera.lookAt));
	glUniform3fv(glGetUniformLocation(prog, "camera.up"), 1, glm::value_ptr(camera.up));
	glUniform3fv(glGetUniformLocation(prog, "camera.right"), 1, glm::value_ptr(camera.right));
	glUniform1f(glGetUniformLocation(prog, "camera.fov"), camera.fov);
	glUniformMatrix4fv(glGetUniformLocation(prog, "camera.cam"), 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(glGetUniformLocation(prog, "time"), _time);

	glBindVertexArray(_vao);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
