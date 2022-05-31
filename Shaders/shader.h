#pragma once

#include <string>

class shader {

	const char* _fpath;
	std::string _ftext;
	GLuint _fShader = 0;
	GLuint _program = 0;

public:

	shader() = delete;
	shader(const char* frag_path);

	static bool init_vert();
	static void destroy_vert();

	~shader();

	void use() const;

	GLuint getProgram() const;

};

