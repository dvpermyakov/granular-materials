#include "Sky.h"

ShaderProgram SkyBox::program;

void SkyBox::loadShaders() {
	program.create();
	program.attachShader(Shader("Resources\\Shaders\\sky\\sky.vert", GL_VERTEX_SHADER));
	program.attachShader(Shader("Resources\\Shaders\\sky\\sky.frag", GL_FRAGMENT_SHADER));
	program.linkProgram();
}

SkyBox::SkyBox() : vao(0) {}

void SkyBox::load() {
	cubeTexture.generate(GL_TEXTURE_CUBE_MAP);
	cubeTexture.addTexture(LEFT_SKYBOX_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	cubeTexture.addTexture(RIGHT_SKYBOX_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	cubeTexture.addTexture(TOP_SKYBOX_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	cubeTexture.addTexture(BOTTOM_SKYBOX_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	cubeTexture.addTexture(BACK_SKYBOX_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	cubeTexture.addTexture(FRONT_SKYBOX_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	cubeTexture.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	cubeTexture.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	cubeTexture.setSamplerParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	cubeTexture.setSamplerParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	vbo.generate();
	vbo.addData(Cube::points, sizeof(Cube::points));
	vbo.bind();
	vbo.upload();

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void SkyBox::render(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, DirectLight light) {
	glBindVertexArray(vao);
	program.useProgram();
	program.setUniform("projectionMatrix", projectionMatrix);
	program.setUniform("viewMatrix", viewMatrix);
	program.setUniform("intensity", light.getIntensity());
	program.setUniform("cubeTexture", 0);
	cubeTexture.bindTexture(0);
	glDepthMask(GL_FALSE);
	for (int i = 0; i < 6; i++) {
		glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
	}
	glDepthMask(GL_TRUE);
}

ShaderProgram Sun::program;

void Sun::loadShaders() {
	program.create();
	program.attachShader(Shader("Resources\\Shaders\\sun\\sun.vert", GL_VERTEX_SHADER));
	program.attachShader(Shader("Resources\\Shaders\\sun\\sun.frag", GL_FRAGMENT_SHADER));
	program.linkProgram();
}

Sun::Sun() : shadowLength(8192) {}

void Sun::load() {
	model.loadModelFromFile(SUN_MODEL_LOCATION);
	model.generateVao();

	shadowMap.init(GL_TEXTURE_2D, shadowLength, shadowLength);
}

void Sun::render(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
	program.useProgram();
	program.setUniform("projectionMatrix", projectionMatrix);
	program.setUniform("viewMatrix", viewMatrix);
	program.setUniform("modelMatrix", glm::translate(glm::mat4(1), 10.0f * light.getPosition()));
	model.bindVao();
	model.renderGeometry();
}

DirectLight* Sun::getLight() {
	return &light;
}

ShadowMap* Sun::getShadowMap() {
	return &shadowMap;
}