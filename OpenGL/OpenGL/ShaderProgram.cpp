#include "ShaderProgram.h"

// Shader

string Shader::codeFromFile(const string& filePath) {
	//open file
	ifstream f;
	f.open(filePath.c_str(), ios::in | ios::binary);
	if (!f.is_open()) {
		throw MessageBox(appMain.hWnd, "File can't be opened", "Error getting shader code from file", MB_ICONERROR);
	}

	//read whole file into stringstream buffer
	stringstream buffer;
	buffer << f.rdbuf();

	f.close();

	//return new shader
	return buffer.str();
}

Shader::Shader(const std::string& filePath, GLenum shaderType) : object(0) {

	string shaderCode = this->codeFromFile(filePath);

	//create the shader object
	object = glCreateShader(shaderType);

	if (object == 0) MessageBox(appMain.hWnd, "Shader is not created", "Error creating shader", MB_ICONERROR);

	//set the source code
	const char* code = shaderCode.c_str();
	glShaderSource(object, 1, (const GLchar**)&code, NULL);

	//compile
	glCompileShader(object);

	//throw exception if compile error occurred
	GLint status;
	glGetShaderiv(object, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		std::string msg("Compile failure in shader (" + filePath + "):\n");

		GLint infoLogLength;
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* strInfoLog = new char[infoLogLength + 1];
		glGetShaderInfoLog(object, infoLogLength, NULL, strInfoLog);
		msg += strInfoLog;
		delete[] strInfoLog;
		glDeleteShader(object);
		object = 0;
		MessageBox(appMain.hWnd, msg.c_str(), "Compilation error in shader program", MB_ICONERROR);
	}
}

GLuint Shader::getObject() const {
	return object;
}

Shader::~Shader() {
	glDeleteShader(object);
	object = 0;
}

// ShaderProgram

ShaderProgram::ShaderProgram() : object(0) {}

void ShaderProgram::create() {
	object = glCreateProgram();
	if (object == 0) MessageBox(appMain.hWnd, "Couldn't create shader program ", "Error glCreateProgram", MB_ICONERROR);
}

void ShaderProgram::attachShader(Shader shader) {
	glAttachShader(object, shader.getObject());
}
void ShaderProgram::linkProgram() {
	glLinkProgram(object);

	GLint status;
	glGetProgramiv(object, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		string msg("Program linking failure: ");

		GLint infoLogLength;
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* strInfoLog = new char[infoLogLength + 1];
		glGetProgramInfoLog(object, infoLogLength, NULL, strInfoLog);
		msg += strInfoLog;
		delete[] strInfoLog;

		glDeleteProgram(object);
		object = 0;
		MessageBox(appMain.hWnd, msg.c_str(), "Error Linking program", MB_ICONERROR);
	}
}

void ShaderProgram::setUniform(string name, const unsigned int value) {
	int location = glGetUniformLocation(object, name.c_str());
	glUniform1ui(location, value);
}

void ShaderProgram::setUniform(string name, const int value) {
	int location = glGetUniformLocation(object, name.c_str());
	glUniform1i(location, value);
}

void ShaderProgram::setUniform(string name, const float value) {
	int location = glGetUniformLocation(object, name.c_str());
	glUniform1fv(location, 1, &value);
}

void ShaderProgram::setUniform(string name, const glm::vec2 value) {
	int location = glGetUniformLocation(object, name.c_str());
	glUniform2fv(location, 1, (GLfloat*)&value);
}

void ShaderProgram::setUniform(string name, const glm::vec3 value) {
	int location = glGetUniformLocation(object, name.c_str());
	glUniform3fv(location, 1, (GLfloat*)&value);
}

void ShaderProgram::setUniform(string name, const glm::vec4 value) {
	int location = glGetUniformLocation(object, name.c_str());
	glUniform4fv(location, 1, (GLfloat*)&value);
}

void ShaderProgram::setUniform(string name, const glm::mat4 value) {
	int location = glGetUniformLocation(object, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, (GLfloat*)&value);
}

void ShaderProgram::useProgram() {
	glUseProgram(object);
}

GLuint ShaderProgram::getAttributeLocation(const GLchar* attributeName) const {
	if (!attributeName) MessageBox(appMain.hWnd, "Attribute was NULL", "Error attribute get location", MB_ICONERROR);

	GLint attribute = glGetAttribLocation(object, attributeName);
	if (attribute == -1) MessageBox(appMain.hWnd, "Attribute was not found", "Error attribute get location", MB_ICONERROR);

	return attribute;
}

GLuint ShaderProgram::getObject() const {
	return object;
}

ShaderProgram::~ShaderProgram() {
	if (object != 0) glDeleteProgram(object);
}