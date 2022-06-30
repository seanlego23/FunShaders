#include "screen.h"
#include "shader_object.h"
#include "shader_inputs.h"

shader_object::shader_object(const char *shader_file) : _mainShader(shader_file), _inputs() { }

shader_inputs* shader_object::get_inputs() {
	return &this->_inputs;
}

void shader_object::mouse_button(int button, int action, int mods) {
	//Behavior is implemented by derived class
}

void shader_object::key_input(int key, int scancode, int action, int mods) {
	//Behavior is implemented by derived class
}

bool shader_object::has_input_shaders() const {
	return false;
}

int shader_object::input_shaders_count() const {
	return 0;
}

bool shader_object::setup_input_shader(const int index) {
	return false;
}
