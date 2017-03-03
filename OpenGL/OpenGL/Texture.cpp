#include "Texture.h"

#include <GL/glew.h>
#include <gl/wglew.h>

Texture::Texture() : isCreated(false), target(NULL), isMipMapsGenerated(false), path("") {}

bool Texture::generate(GLenum target, bool isGenerateMipMaps) {
	if (isCreated) return false;

	glGenTextures(1, &object);
	glBindTexture(target, object);

	if (isGenerateMipMaps) {
		glGenerateMipmap(target);
	}

	glGenSamplers(1, &sampler);

	this->target = target;
	isCreated = true;

	return true;
}

bool Texture::addTexture(string path, GLenum target) {
	if (!isCreated) return false;

	FIBITMAP* dib = loadData(path);
	if (!dib) return false;

	BYTE* dataPointer = FreeImage_GetBits(dib);

	int width = FreeImage_GetWidth(dib); 
	int height = FreeImage_GetHeight(dib);
	int BPP = FreeImage_GetBPP(dib);

	if (dataPointer == NULL || width == 0 || height == 0) return false;

	int format = BPP == 24 ? GL_BGR : BPP == 8 ? GL_LUMINANCE : 0;
	int internalFormat = BPP == 24 ? GL_RGB : GL_DEPTH_COMPONENT;

	glTexImage2D(target, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, dataPointer);

	return true;
}


bool Texture::addTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum target) {
	if (!isCreated) return false;

	glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);

	return true;
}

bool Texture::create(int width, int height, GLenum internalFormat, GLenum format, GLenum type, bool isGenerateMipMaps, GLenum target) {
	if (isCreated) return false;

	glGenTextures(1, &object);
	glBindTexture(target, object);
	glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);

	if (isGenerateMipMaps) {
		glGenerateMipmap(target);
	}

	glGenSamplers(1, &sampler);

	this->target = target;
	this->isMipMapsGenerated = isGenerateMipMaps;
	this->width = width;
	this->height = height;
	isCreated = true;

	return true;
}

bool Texture::loadFromData1D(void* data, int size, GLenum internalFormat, GLenum format, GLenum type, bool isGenerateMipMaps, GLenum target) {
	if (isCreated) return false;

	glGenTextures(1, &object);
	glBindTexture(target, object);

	glTexImage1D(target, 0, internalFormat, size, 0, format, type, data);

	if (isGenerateMipMaps) {
		glGenerateMipmap(target);
	}

	glGenSamplers(1, &sampler);

	this->target = target;
	this->isMipMapsGenerated = isGenerateMipMaps;
	this->width = size;
	this->height = size;
	isCreated = true;

	return true;
}

bool Texture::loadFromData(BYTE* data, int width, int height, int BPP, GLenum format, bool isGenerateMipMaps, GLenum target) {
	if (isCreated) return false;

	glGenTextures(1, &object);
	glBindTexture(target, object);

	if (format == GL_RGB || format == GL_BGR) // We must handle this because of internal format parameter
	{
		glTexImage2D(target, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		if (isGenerateMipMaps) glGenerateMipmap(target);
	}
	else
	{
		glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		if (isGenerateMipMaps) glGenerateMipmap(target);
	}
	
	glGenSamplers(1, &sampler);

	this->target = target;
	this->isMipMapsGenerated = isGenerateMipMaps;
	this->width = width;
	this->height = height;
	this->BPP = BPP;
	isCreated = true;

	return true;
}

bool Texture::loadFromPath(string path, bool isGenerateMipMaps, GLenum target)
{
	if (isCreated) return false;

	FIBITMAP* dib = loadData(path);
	if (!dib) return false;

	BYTE* dataPointer = FreeImage_GetBits(dib);

	width = FreeImage_GetWidth(dib); // Get the image width, height and bytes per pixel
	height = FreeImage_GetHeight(dib);
	BPP = FreeImage_GetBPP(dib);

	if (dataPointer == NULL || width == 0 || height == 0) return false;

	glGenTextures(1, &object);
	glBindTexture(target, object);

	int format = BPP == 24 ? GL_BGR : BPP == 8 ? GL_LUMINANCE : 0;
	int internalFormat = BPP == 24 ? GL_RGB : GL_DEPTH_COMPONENT;

	glTexImage2D(target, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, dataPointer);

	if (isGenerateMipMaps) {
		glGenerateMipmap(target);
	}

	FreeImage_Unload(dib);

	glGenSamplers(1, &sampler);

	this->target = target;
	this->path = path;
	this->isMipMapsGenerated = isGenerateMipMaps;
	isCreated = true;

	return true;
}

void Texture::setSamplerParameter(GLenum parameter, GLenum value) {
	glSamplerParameteri(sampler, parameter, value);
}

void Texture::setSamplerParameter(GLenum parameter, GLfloat* value) {
	glSamplerParameterfv(sampler, parameter, value);
}

void Texture::bindTexture(int textureUnit) {
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(target, object);
	glBindSampler(textureUnit, sampler);
}

void Texture::unbindTexture(int textureUnit) {
	glBindSampler(textureUnit, 0);
}

void Texture::releaseTexture() {
	glDeleteTextures(1, &object);
	glDeleteSamplers(1, &sampler);
	isCreated = false;
}

string Texture::getPath() {
	return path;
}

GLuint Texture::getObject() {
	return object;
}

GLenum Texture::getTarget() {
	return target;
}

int Texture::getWidth() {
	return width;
}

int Texture::getHeight() {
	return height;
}

bool Texture::wasCreated() {
	return isCreated;
}

FIBITMAP* Texture::loadData(string path) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* dib(0);

	fif = FreeImage_GetFileType(path.c_str(), 0); // Check the file signature and deduce its format

	if (fif == FIF_UNKNOWN) // If still unknown, try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(path.c_str());

	if (fif == FIF_UNKNOWN) // If still unkown, return failure
		return NULL;

	if (FreeImage_FIFSupportsReading(fif)) // Check if the plugin has reading capabilities and load the file
		dib = FreeImage_Load(fif, path.c_str());

	return dib;
}