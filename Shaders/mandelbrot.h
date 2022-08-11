#pragma once

#include <glad/glad.h>

#include "shader_object.h"

class mandelbrot : public shader_object {

	shader_inputs _inputs;

public:

	mandelbrot();

	mandelbrot(shader_inputs&& inputs);

	~mandelbrot() override = default;

	shader_inputs* get_inputs() override;

	void scroll_input(screen* scr, double xoffset, double yoffset) override;

	void pan_input(screen* scr, double xpos, double ypos) override;

	void rotate_input(screen* scr, double xpos, double ypos) override { }

	bool has_input_shaders() const override;

	int input_shaders_count() const override;

	GLuint setup_input_shader(const int index) override;

	void framebuffer_resize(int width, int height) { }

};

