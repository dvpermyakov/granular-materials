#pragma once

#include "Buffer.h"
#include "ShaderProgram.h"
#include "Light.h"
#include "Camera.h"
#include "Texture.h"
#include "Resources.h"
#include "Environment.h"

#include <string>
#include <vector>

using namespace std;

class Ground : public AbstractModel {
public:
	static void loadShaders();
	Ground();
	void load();
	void renderGeometry();
	void render(Matricies matricies, Camera* camera, Lights lights, bool withAnimation = false); 
	float getHeight(float x, float y);
	glm::mat4 getModelMatrix();
	glm::vec3 getScale();
	int getDataHeight();
	int getDataWidth();
	BYTE* getData();
	BYTE getRowHeight(int x, int y);
private:
	static ShaderProgram program;
	ShaderProgram* getProgram();
	bool loadHeightMap(string path);
	BYTE* dataPointer;
	GLuint vao;
	VertexBufferObject vbo;
	VertexBufferObject indices;
	int height;
	int width;
	int bytePerPixel;

	glm::vec3 scale;
	float texturePrecision;
	Texture highTexture;
	Texture middleTexture;
	Texture lowTexture;
	float highMin;
	float middleMin;
	float lowMin;
};