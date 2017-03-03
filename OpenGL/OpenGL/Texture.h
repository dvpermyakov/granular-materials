#pragma once

#include <string>
#include <windows.h>

#include <GL/glew.h>
#include <gl/wglew.h>

#include <FreeImage.h>

using namespace std;

class Texture {
public:
	Texture();
	bool generate(GLenum target, bool isGenerateMipMaps = false);                                                                             
	bool addTexture(string path, GLenum target);
	bool addTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum target = GL_TEXTURE_2D);
	bool create(int width, int height, GLenum internalFormat, GLenum format, GLenum type, bool isGenerateMipMaps = false, GLenum target = GL_TEXTURE_2D);
	bool loadFromData1D(void* data, int size, GLenum internalFormat, GLenum format, GLenum type, bool isGenerateMipMaps = false, GLenum target = GL_TEXTURE_1D);
	bool loadFromPath(string path, bool isGenerateMipMaps = false, GLenum target = GL_TEXTURE_2D);
	bool loadFromData(BYTE* data, int width, int height, int BPP, GLenum format, bool isGenerateMipMaps = false, GLenum target = GL_TEXTURE_2D);
	void bindTexture(int textureUnit = 0);
	void unbindTexture(int textureUnit = 0);
	void setSamplerParameter(GLenum parameter, GLenum value);
	void setSamplerParameter(GLenum parameter, GLfloat* value);
	int getMinificationFilter();
	int getMagnificationFilter();
	string getPath();
	GLuint getObject();
	GLenum getTarget();
	int getWidth();
	int getHeight();
	bool wasCreated();
	void releaseTexture();
	static FIBITMAP* loadData(string path);
private:
	bool isCreated;
	int width, height, BPP; // Texture width, height, and bytes per pixel
	GLenum target;
	GLuint object;
	GLuint sampler;
	bool isMipMapsGenerated;
	int minification, magnification, wrap;  // filtres
	string path;
};