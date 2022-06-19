#pragma once

class shader_object {

	class shader* _shader;

public:

	struct shader_inputs* inputs;

	void move_cursor(double x, double y);

	void pan(struct screen* scr, double x, double y);

	void zoom(struct screen* scr, double x, double y);

	void rotate(struct screen* scr, double x, double y);

	virtual void mouse_button(int button, int action, int mods);

	virtual void key_input(int key, int scancode, int action, int mods);

};

