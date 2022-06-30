#pragma once

#include "shader.h"
#include "shader_inputs.h"

class shader_object {

	shader _mainShader;
	shader_inputs _inputs;

protected:

	explicit shader_object(const char* shader_file);

public:

	shader_object() = delete;

	virtual ~shader_object() = default;

	shader_inputs* get_inputs();

	virtual void mouse_button(int button, int action, int mods);

	virtual void key_input(int key, int scancode, int action, int mods);

	virtual bool has_input_shaders() const = 0;

	virtual int input_shaders_count() const = 0;

	virtual bool setup_input_shader(const int index) = 0;

};

