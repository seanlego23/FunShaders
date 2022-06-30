#include "mandelbowl.h"

mandelbowl::mandelbowl() : shader_object("data/mandelbowl.glsl") {

}

bool mandelbowl::has_input_shaders() const {
	return true;
}

int mandelbowl::input_shaders_count() const {
	return 1;
}

bool mandelbowl::setup_input_shader(const int index) {
	return false;
}
