#pragma once

#include "Buffer.h"
#include "ShaderProgram.h"
#include "Light.h"

class ShadowMap {
public:
	ShadowMap();
	static void loadShaders();
	void init(GLenum target, int width, int height);
	void bind(bool granular = false);
	void unbind();
	void anchorTexture();
	void removeTexture();
	void setViewProjectionMatrices(DirectLight* light, bool granular = false);
	void setViewProjectionMatrices(PointLight* light);
	void setModelMatrix(glm::mat4 modelMatrix, bool granular = false);
	Texture* getTexture();
private:
	static ShaderProgram program;
	static ShaderProgram cubeProgram;
	static ShaderProgram granuleProgram;
	int width;
	int height;
	bool wasAnchored;
	FrameBufferObject fbo;
	GLenum target;
};

class VolumeShadow {
public:
	static void loadShaders();
	static ShaderProgram zBufferProgram;
	static ShaderProgram stencilProgram;
};