#pragma once

#include <glad/glad.h>

#include "shader_inputs.h"
#include "shader_object.h"

class mandelbowl : public shader_object {

	struct mandelbowl_inputs : public shader_inputs {
		GLuint partTexture = 0;
		GLuint normTexture = 0;
		GLuint maskTexture = 0;

		mandelbowl_inputs() { }

	};

	shader _partShader;
	shader _normShader;
	mandelbowl_inputs _inputs;

	GLuint _partFB;
	GLuint _normFB;

	void init();

public:

	mandelbowl();
	
	mandelbowl(shader_inputs&& inputs);

	~mandelbowl() override;

	shader_inputs* get_inputs();

	bool has_input_shaders() const override;

	int input_shaders_count() const override;

	GLuint setup_input_shader(const int index) override;

	void framebuffer_resize(int width, int height) override;

};

