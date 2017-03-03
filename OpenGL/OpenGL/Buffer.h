#pragma once

#include "Texture.h"
#include "WindowApp.h"

#include <GL/glew.h>
#include <gl/wglew.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <vector>

using namespace std;

class VertexBufferObject {
public:
	VertexBufferObject();
	VertexBufferObject(VertexBufferObject& other);
	void generate();
	void bind(GLenum type = GL_ARRAY_BUFFER);
	void upload(GLenum usage = GL_STATIC_DRAW, bool clear = true);
	void addData(void* pointer, UINT size);
	void changeData(void* pointer, UINT size, UINT offset);
	int getSize();
	void deleteVBO();
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version) { ar & data; ar & size; }
	GLuint object;
	GLenum type;
	GLenum usage;
	vector<BYTE> data;
	int size;
};

class FrameBufferObject {
public:
	FrameBufferObject();
	void generate();
	bool createTexture(GLenum target, GLenum internalFormat, GLenum format, GLenum type, int width = 0, int height = 0);
	bool addTexture(GLenum target, GLenum format, int width, int height);
	void bind(GLenum target, GLenum attachment);
	void unbind();
	GLuint getObject();
	Texture* getTexture();
	void deleteFBO();
private:
	GLenum format;
	int width, height;
	GLuint object;
	Texture texture;
};