#pragma once

class object {

	class shader* _shader;
	struct shader_inputs* _inputs;

public:

	virtual void move_cursor(double x, double y);

	virtual void zoom(double x, double y);

	virtual void mouse_button(int button, int action, int mods);

	virtual void key_input(int key, int scancode, int action, int mods);

};

