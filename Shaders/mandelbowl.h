#pragma once

#include "shader_object.h"

class mandelbowl : public shader_object {

	shader _partShader;

public:

	mandelbowl();
	~mandelbowl() override = default;

	bool has_input_shaders() const override;

	int input_shaders_count() const override;

	bool setup_input_shader(const int index) override;

};

