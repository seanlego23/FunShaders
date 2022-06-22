#pragma once

#include "shader.h"
#include "shader_inputs.h"

class shader_object : public shader {
protected:

	shader_object() = delete;
	explicit shader_object(const char* shader_file);

public:

	shader_inputs inputs;

	virtual ~shader_object() = default;

	void move_cursor(double x, double y);

	void pan(class screen* scr, double x, double y);

	void zoom(class screen* scr, double x, double y);

	void rotate(class screen* scr, double x, double y);

	virtual void mouse_button(int button, int action, int mods);

	virtual void key_input(int key, int scancode, int action, int mods);

	virtual void setup_render() = 0;
};

