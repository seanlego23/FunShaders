#include "screen.h"
#include "shader_object.h"
#include "shader_inputs.h"

shader_object::shader_object(const char *shader_file) : shader(shader_file), inputs() { }

void shader_object::move_cursor(double x, double y) {
	inputs.cursorPos = glm::vec2((float)x, (float)y);
}

void shader_object::pan(screen* scr, double x, double y) {
	
}

void shader_object::zoom(screen* scr, double x, double y) {

}

void shader_object::rotate(screen* scr, double x, double y) {

}

void shader_object::mouse_button(int button, int action, int mods) {

}

void shader_object::key_input(int key, int scancode, int action, int mods) {

}
