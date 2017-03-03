#pragma once

#include "Texture.h"
#include "Buffer.h"
#include "ShaderProgram.h"
#include "Model.h"
#include "Light.h"
#include "Shadow.h"
#include "Resources.h"
#include "Geometry.h"

#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <gl/wglew.h>

using namespace std;

class SkyBox {
public:
	static void loadShaders();
	SkyBox();
	void load();
	void render(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, DirectLight light);
private:
	static ShaderProgram program;
	GLuint vao;
	VertexBufferObject vbo;
	Texture cubeTexture;
};

class Sun {
public:
	static void loadShaders();
	Sun();
	void load();
	void render(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	DirectLight* getLight();
	ShadowMap* getShadowMap();
private:
	static ShaderProgram program;
	Model model;
	DirectLight light;
	ShadowMap shadowMap;
	int shadowLength;
};