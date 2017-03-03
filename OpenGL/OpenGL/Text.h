#pragma once

#include "ShaderProgram.h"
#include "Buffer.h"
#include "Texture.h"

#include <ft2build.h>
#include <ftheader.h>
#include FT_FREETYPE_H

#include <Windows.h>
#include <string>
#include <algorithm> 

using namespace std;

class GLFont {
public:
	static void loadShaders();

	GLFont();
	bool loadFont(string filePath, int PXSize);
	bool loadSystemFont(string name, int PXSize);
	void print(glm::mat4 projectionMatrix, string text, glm::vec3 color, int x, int y, int PXSize = -1);  // if PXSize is -1 then we use this->pixelSize
	int getLength(string text, int PXSize = -1);
	void releaseFont();
private:
	static ShaderProgram program;
	
	void createChar(int index);
	Texture charTextures[256];
	int iAdvX[256], iAdvY[256];
	int iBearingX[256], iBearingY[256];
	int iCharWidth[256], iCharHeight[256];
	int pixelSize;
	int iNewLine;
	GLuint vao;
	VertexBufferObject vbo;
	FT_Library ftLib;
	FT_Face ftFace;
};