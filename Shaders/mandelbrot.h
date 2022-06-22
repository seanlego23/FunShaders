#pragma once

#include "shader_object.h"

class mandelbrot : public shader_object {
public:

	mandelbrot();
	~mandelbrot() override = default;

	void setup_render() override;

};

