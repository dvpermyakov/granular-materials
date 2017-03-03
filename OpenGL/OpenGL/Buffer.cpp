#include "Buffer.h"

VertexBufferObject::VertexBufferObject() : object(0), size(0) {}

VertexBufferObject::VertexBufferObject(VertexBufferObject& other) : VertexBufferObject() {
	generate();
	addData(&other.data[0], other.getSize());
}

void VertexBufferObject::generate() {
	glGenBuffers(1, &object);
}

void VertexBufferObject::bind(GLenum type) {
	this->type = type;
	glBindBuffer(type, object);
}

void VertexBufferObject::upload(GLenum usage, bool clear) {
	this->usage = usage;
	glBufferData(type, data.size(), &data[0], usage);
	if (clear) {
		data.clear();
		size = 0;
	}
}

void VertexBufferObject::addData(void* pointer, UINT size) {
	data.insert(data.end(), (BYTE*)pointer, (BYTE*)pointer + size);
	this->size += size;
}

void VertexBufferObject::changeData(void* pointer, UINT size, UINT offset) {
	if (usage == GL_DYNAMIC_DRAW) {
		glBindBuffer(type, object);
		glBufferSubData(type, offset, size, (BYTE*)pointer);
	}
}

int VertexBufferObject::getSize() {
	return size;
}

void VertexBufferObject::deleteVBO() {
	glDeleteBuffers(1, &object);
	data.clear();
}

// FrameBufferObject methods

FrameBufferObject::FrameBufferObject() : object(0) {}

void FrameBufferObject::generate() {
	glGenFramebuffers(1, &object);
}

bool FrameBufferObject::createTexture(GLenum target, GLenum internalFormat, GLenum format, GLenum type, int width, int height) {
	if (target == GL_TEXTURE_2D) texture.create(width, height, internalFormat, format, type);
	if (target == GL_TEXTURE_CUBE_MAP) texture.generate(GL_TEXTURE_CUBE_MAP);
	return true;
}

bool FrameBufferObject::addTexture(GLenum target, GLenum format, int width, int height) {
	//if (this->texture.getTarget() != GL_TEXTURE_CUBE_MAP) return false;
	texture.addTexture(width, height, format, format, GL_FLOAT, target);
	return true;
}

void FrameBufferObject::bind(GLenum target, GLenum attachment) {
	glBindFramebuffer(target, object);
	glFramebufferTexture(target, attachment, texture.getObject(), 0);
}

void FrameBufferObject::unbind() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Texture* FrameBufferObject::getTexture() {
	return &texture;
}

GLuint FrameBufferObject::getObject() {
	return object;
}

void FrameBufferObject::deleteFBO() {

}