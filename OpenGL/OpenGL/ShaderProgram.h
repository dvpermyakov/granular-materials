#pragma once

#include "WindowApp.h"
//#include "Shader.h"

#include <GL/glew.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

class Shader {
public:
	Shader(const string& filePath, GLenum shaderType);
	~Shader();
	GLuint getObject() const;
private:
	static std::string codeFromFile(const string& filePath);
	GLuint object;
};

class ShaderProgram {
public:
	ShaderProgram();
	~ShaderProgram();
	void create();
	void attachShader(Shader shader);
	void linkProgram();
	void setUniform(string name, const unsigned int value);
	void setUniform(string name, const int value);
	void setUniform(string name, const float value);
	void setUniform(string name, const glm::vec2 value);
	void setUniform(string name, const glm::vec3 value);
	void setUniform(string name, const glm::vec4 value);
	void setUniform(string name, const glm::mat4 value);
	void useProgram();
	GLuint getObject() const;
	GLuint getAttributeLocation(const GLchar* attributeName) const;
private:
	GLuint object;
};