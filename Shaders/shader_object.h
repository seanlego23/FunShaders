#pragma once

#include "shader.h"
#include "shader_inputs.h"

class shader_object : public shader {
protected:

	shader_object() = delete;
	explicit shader_object(const char* shader_file);

public:

	shader_inputs inputs;

	~shader_object() override = default;

	virtual void mouse_button(int button, int action, int mods);

	virtual void key_input(int key, int scancode, int action, int mods);

	virtual void setup_render() = 0;
};

