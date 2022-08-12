#include <glad/glad.h>

#include "mandelbrot.h"

mandelbrot::mandelbrot() : shader_object("data/mandelbrot.glsl") {

}

mandelbrot::mandelbrot(shader_inputs&& inputs) : shader_object("data/mandelbrot.glsl"), _inputs(std::move(inputs)) {

}

shader_inputs* mandelbrot::get_inputs() {
	return &_inputs;
}

bool mandelbrot::has_input_shaders() const {
	return false;
}

int mandelbrot::input_shaders_count() const {
	return 0;
}

GLuint mandelbrot::setup_input_shader(const int index) {
	return 0;
}