#pragma once

#include <glad/glad.h>

#include "shader.h"
#include "shader_inputs.h"

class screen;

class shader_object {

	shader _mainShader;

protected:

	explicit shader_object(const char* shader_file);

public:

	shader_object() = delete;

	virtual ~shader_object() = default;

	virtual shader_inputs* get_inputs() = 0;

	virtual void mouse_button(screen* scr, int button, int action, int mods);

	virtual void key_input(screen* scr, int key, int scancode, int action, int mods);

	virtual void scroll_input(screen* scr, double xoffset, double yoffset) = 0;

	virtual void pan_input(screen* scr, double xpos, double ypos) = 0;

	virtual void rotate_input(screen* scr, double xpos, double ypos) = 0;

	virtual bool has_input_shaders() const = 0;

	virtual int input_shaders_count() const = 0;

	virtual GLuint setup_input_shader(const int index) = 0;

	virtual void framebuffer_resize(int width, int height) = 0;

	virtual GLuint use_main_program() final;

};

