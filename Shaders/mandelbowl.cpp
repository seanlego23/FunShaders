#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>

#include "mandelbowl.h"
#include "screen.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

void mandelbowl::init() {
	glGenTextures(1, &_inputs.partTexture);
	glGenTextures(1, &_inputs.normTexture);
	glGenTextures(1, &_inputs.maskTexture);

	glBindTexture(GL_TEXTURE_2D, _inputs.partTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8I, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED_INTEGER, GL_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, _inputs.normTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, _inputs.maskTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8I, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED_INTEGER, GL_BYTE, NULL);

	glGenFramebuffers(1, &_partFB);
	glGenFramebuffers(1, &_normFB);

	glBindFramebuffer(GL_FRAMEBUFFER, _partFB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _inputs.partTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

	glBindFramebuffer(GL_FRAMEBUFFER, _normFB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _inputs.normTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _inputs.maskTexture, 0);

	unsigned int attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_normShader.use();
	glUniform1i(glGetUniformLocation(_normShader.getProgram(), "part_tex"), 0);

	GLuint mprog = this->use_main_program();
	glUniform1i(glGetUniformLocation(mprog, "part_tex"), 0);
	glUniform1i(glGetUniformLocation(mprog, "norm_tex"), 1);
	glUniform1i(glGetUniformLocation(mprog, "mask_tex"), 2);
}

mandelbowl::mandelbowl() : shader_object("data/mandelbowl.glsl"),
	_partShader("data/mandelbowl_parts.glsl"), _normShader("data/mandelbowl_normals.glsl") {
	init();
}

mandelbowl::mandelbowl(shader_inputs&& inputs) : shader_object("data/mandelbowl.glsl"),
	_partShader("data/mandelbowl_parts.glsl"), _normShader("data/mandelbowl_normals.glsl") {
	init();
	_inputs.elapsedTime = inputs.elapsedTime;
	_inputs.zoom = inputs.zoom;
	_inputs.zoomRaw = inputs.zoomRaw;
}

mandelbowl::~mandelbowl() {
	glDeleteTextures(1, &_inputs.partTexture);
	glDeleteTextures(1, &_inputs.normTexture);
	glDeleteTextures(1, &_inputs.maskTexture);
	glDeleteFramebuffers(1, &_partFB);
	glDeleteFramebuffers(1, &_normFB);
}

shader_inputs* mandelbowl::get_inputs() {
	return &_inputs;
}

void mandelbowl::pan_input(screen* scr, double xpos, double ypos) {

}

void mandelbowl::rotate_input(screen* scr, double xpos, double ypos) {
	
}

bool mandelbowl::has_input_shaders() const {
	return true;
}

int mandelbowl::input_shaders_count() const {
	return 2;
}

GLuint mandelbowl::setup_input_shader(const int index) {

	switch (index) {
		case 0:
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, _inputs.partTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, _inputs.normTexture);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, _inputs.maskTexture);

			glDisable(GL_DITHER);

			glBindFramebuffer(GL_FRAMEBUFFER, _partFB);
			glClear(GL_COLOR_BUFFER_BIT);
			_partShader.use();
			return _partShader.getProgram();
		}
		case 1:
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _normFB);
			glClear(GL_COLOR_BUFFER_BIT);
			_normShader.use();

			return _normShader.getProgram();
		}
		default:
		{
			return 0;
		}
	}
}

void mandelbowl::framebuffer_resize(int width, int height) {
	glDeleteTextures(1, &_inputs.partTexture);
	glDeleteTextures(1, &_inputs.normTexture);
	glDeleteTextures(1, &_inputs.maskTexture);

	glGenTextures(1, &_inputs.partTexture);
	glGenTextures(1, &_inputs.normTexture);
	glGenTextures(1, &_inputs.maskTexture);

	glBindTexture(GL_TEXTURE_2D, _inputs.partTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8I, width, height, 0, GL_RED_INTEGER, GL_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, _inputs.normTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, _inputs.maskTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8I, width, height, 0, GL_RED_INTEGER, GL_BYTE, NULL);

	glBindFramebuffer(GL_FRAMEBUFFER, _partFB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _inputs.partTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, _normFB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _inputs.normTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _inputs.maskTexture, 0);

	unsigned int attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
