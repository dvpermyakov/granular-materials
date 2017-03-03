#include "Shadow.h"

ShaderProgram ShadowMap::program;
ShaderProgram ShadowMap::cubeProgram;
ShaderProgram ShadowMap::granuleProgram;

void ShadowMap::loadShaders() {
	program.create();
	program.attachShader(Shader("Resources\\Shaders\\shadows\\shadow_map.vert", GL_VERTEX_SHADER));
	program.attachShader(Shader("Resources\\Shaders\\shadows\\shadow_map.frag", GL_FRAGMENT_SHADER));
	program.linkProgram();

	cubeProgram.create();
	cubeProgram.attachShader(Shader("Resources\\Shaders\\shadows\\shadow_map_cube.vert", GL_VERTEX_SHADER));
	cubeProgram.attachShader(Shader("Resources\\Shaders\\shadows\\shadow_map_cube.geom", GL_GEOMETRY_SHADER));
	cubeProgram.attachShader(Shader("Resources\\Shaders\\shadows\\shadow_map_cube.frag", GL_FRAGMENT_SHADER));
	cubeProgram.linkProgram();

	granuleProgram.create();
	granuleProgram.attachShader(Shader("Resources\\Shaders\\shadows\\shadow_map_granule.vert", GL_VERTEX_SHADER));
	granuleProgram.attachShader(Shader("Resources\\Shaders\\shadows\\shadow_map_granule.frag", GL_FRAGMENT_SHADER));
	granuleProgram.linkProgram();
}

ShadowMap::ShadowMap() : wasAnchored(false) {}

void ShadowMap::init(GLenum target, int width, int height) {
	fbo.generate();
	if (target == GL_TEXTURE_2D) {
		fbo.createTexture(target, GL_RGB, GL_RGB, GL_FLOAT, width, height);
	}
	if (target == GL_TEXTURE_CUBE_MAP) {
		fbo.createTexture(target, GL_RGB, GL_RGB, GL_FLOAT);
		fbo.addTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGB, width, height);
		fbo.addTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_RGB, width, height);
		fbo.addTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_RGB, width, height);
		fbo.addTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_RGB, width, height);
		fbo.addTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_RGB, width, height);
		fbo.addTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_RGB, width, height);
	}
	fbo.getTexture()->setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	fbo.getTexture()->setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	fbo.getTexture()->setSamplerParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	fbo.getTexture()->setSamplerParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	fbo.getTexture()->setSamplerParameter(GL_TEXTURE_BORDER_COLOR, borderColor);

	this->target = target;
	this->width = width;
	this->height = height;
}

void ShadowMap::bind(bool granular) {
	fbo.bind(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, width, height);
	if (!granular) {
		if (target == GL_TEXTURE_2D) program.useProgram();
		if (target == GL_TEXTURE_CUBE_MAP) cubeProgram.useProgram();
	}
	else {
		granuleProgram.useProgram();
	}
}


void ShadowMap::setViewProjectionMatrices(DirectLight* light, bool granular) {
	if (!granular) {
		program.setUniform("projectionMatrix", light->getProjectionMatrix());
		program.setUniform("viewMatrix", light->getViewMatrix());
	}
	else {
		granuleProgram.setUniform("projectionMatrix", light->getProjectionMatrix());
		granuleProgram.setUniform("viewMatrix", light->getViewMatrix());
	}
}

void ShadowMap::setViewProjectionMatrices(PointLight* light) {
	for (int i = 0; i < 6; i++) {
		cubeProgram.setUniform("shadowMatrices[" + to_string(i) + "]", light->getProjectionMatrix() * light->getViewMatrix(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i));
	}
	cubeProgram.setUniform("lightPosition", light->getPosition());
	cubeProgram.setUniform("projectionFarPlane", light->getFarPlane());
}

void ShadowMap::setModelMatrix(glm::mat4 modelMatrix, bool granular) {
	if (!granular) {
		if (target == GL_TEXTURE_2D) program.setUniform("modelMatrix", modelMatrix);
		if (target == GL_TEXTURE_CUBE_MAP) cubeProgram.setUniform("modelMatrix", modelMatrix);
	}
	else {
		granuleProgram.setUniform("modelMatrix", modelMatrix);
	}
}

void ShadowMap::unbind() {
	fbo.unbind();
	glViewport(0, 0, appMain.getWidth(), appMain.getHeight());
}

void ShadowMap::anchorTexture() {
	wasAnchored = true;
}

void ShadowMap::removeTexture() {
	wasAnchored = false;
}

Texture* ShadowMap::getTexture() {
	if (wasAnchored) return fbo.getTexture();
	else return NULL;
}

ShaderProgram VolumeShadow::zBufferProgram;
ShaderProgram VolumeShadow::stencilProgram;

void VolumeShadow::loadShaders() {
	zBufferProgram.create();
	zBufferProgram.attachShader(Shader("Resources\\Shaders\\volume_shadows\\z.vert", GL_VERTEX_SHADER));
	zBufferProgram.attachShader(Shader("Resources\\Shaders\\volume_shadows\\z.frag", GL_FRAGMENT_SHADER));
	zBufferProgram.linkProgram();

	stencilProgram.create();
	stencilProgram.attachShader(Shader("Resources\\Shaders\\volume_shadows\\stencil.vert", GL_VERTEX_SHADER));
	stencilProgram.attachShader(Shader("Resources\\Shaders\\volume_shadows\\stencil.geom", GL_GEOMETRY_SHADER));
	stencilProgram.attachShader(Shader("Resources\\Shaders\\volume_shadows\\stencil.frag", GL_FRAGMENT_SHADER));
	stencilProgram.linkProgram();
}