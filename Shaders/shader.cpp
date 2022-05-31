#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glad/glad.h>

#include "shader.h"

static const char* vpath = "data/vert.glsl";
static std::string vtext;
static GLuint vShader = 0;

bool checkCompileErrors(GLuint shader, std::string type);

shader::shader(const char *frag_path) {
	_fpath = frag_path;

	std::ifstream fShaderFile;
	std::stringstream fShaderStream;

	fShaderFile.open(_fpath);
	if (!fShaderFile.is_open()) {
		std::cout << "Fragment Shader File failed to open\n";
		return;
	}

	fShaderStream << fShaderFile.rdbuf();
	fShaderFile.close();
	_ftext = fShaderStream.str();
	const char* cftext = _ftext.c_str();

	_fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(_fShader, 1, &cftext, NULL);
	glCompileShader(_fShader);
	checkCompileErrors(_fShader, "FRAGMENT");

	_program = glCreateProgram();
	glAttachShader(_program, vShader);
	glAttachShader(_program, _fShader);
	glLinkProgram(_program);
	checkCompileErrors(_program, "PROGRAM");
}

bool shader::init_vert() {

	std::ifstream vShaderFile;
	std::stringstream vShaderStream;

	vShaderFile.open("data/vert.glsl");
	if (!vShaderFile.is_open()) {
		std::cout << "Vertex Shader File failed to open\n";
		return false;
	}

	vShaderStream << vShaderFile.rdbuf();
	vShaderFile.close();
	vtext = vShaderStream.str();
	const char* cvtext = vtext.c_str();

	vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &cvtext, NULL);
	glCompileShader(vShader);
	return checkCompileErrors(vShader, "VERTEX");
}

void shader::destroy_vert() {
	if (vShader)
		glDeleteShader(vShader);
}

shader::~shader() {
	glDeleteShader(_fShader);
	glDeleteProgram(_program);
}

GLuint shader::getProgram() const {
	return _program;
}

void shader::use() const {
	glUseProgram(_program);
}

bool checkCompileErrors(GLuint shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			return false;
        }
    }
	return true;
}
