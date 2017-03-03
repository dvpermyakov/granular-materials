#include "Text.h"

int getNearest2Pow(int value) {
	int pow = 1;
	while (value > pow) {
		pow *= 2;
	}
	return pow;
}

ShaderProgram GLFont::program;

void GLFont::loadShaders() {
	program.create();
	program.attachShader(Shader("Resources\\Shaders\\text\\text.vert", GL_VERTEX_SHADER));
	program.attachShader(Shader("Resources\\Shaders\\text\\text.frag", GL_FRAGMENT_SHADER));
	program.linkProgram();
}

GLFont::GLFont() : vao(0) {}

void GLFont::createChar(int index) {  // unicode index 
	FT_Load_Glyph(ftFace, FT_Get_Char_Index(ftFace, index), FT_LOAD_DEFAULT);

	FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);
	FT_Bitmap* pBitmap = &ftFace->glyph->bitmap;

	int iW = pBitmap->width, iH = pBitmap->rows;
	int iTW = getNearest2Pow(iW), iTH = getNearest2Pow(iH);

	GLubyte* bData = new GLubyte[iTW*iTH];
	// Copy glyph data and add dark pixels elsewhere
	for (int ch = 0; ch < iTH; ch++) {
		for (int cw = 0; cw < iTW; cw++)
			bData[ch*iTW + cw] = (ch >= iH || cw >= iW) ? 0 : pBitmap->buffer[(iH - ch - 1)*iW + cw];
	}

	// And create a texture from it
	charTextures[index].loadFromData(bData, iTW, iTH, 8, GL_DEPTH_COMPONENT, false);
	charTextures[index].setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	charTextures[index].setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	charTextures[index].setSamplerParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	charTextures[index].setSamplerParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Calculate glyph data
	iAdvX[index] = ftFace->glyph->advance.x >> 6;
	iBearingX[index] = ftFace->glyph->metrics.horiBearingX >> 6;
	iCharWidth[index] = ftFace->glyph->metrics.width >> 6;

	iAdvY[index] = (ftFace->glyph->metrics.height - ftFace->glyph->metrics.horiBearingY) >> 6;
	iBearingY[index] = ftFace->glyph->metrics.horiBearingY >> 6;
	iCharHeight[index] = ftFace->glyph->metrics.height >> 6;

	iNewLine = max(iNewLine, int(ftFace->glyph->metrics.height >> 6));

	// Rendering data, texture coordinates are always the same, so now we waste a little memory
	glm::vec2 vQuad[] =
	{
		glm::vec2(0.0f, float(-iAdvY[index] + iTH)),
		glm::vec2(0.0f, float(-iAdvY[index])),
		glm::vec2(float(iTW), float(-iAdvY[index] + iTH)),
		glm::vec2(float(iTW), float(-iAdvY[index]))
	};
	glm::vec2 vTexQuad[] = { glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f) };

	// Add this char to VBO
	for (int i = 0; i < 4; i++) {
		vbo.addData(&vQuad[i], sizeof(glm::vec2));
		vbo.addData(&vTexQuad[i], sizeof(glm::vec2));
	}

	delete[] bData;
}

bool GLFont::loadFont(string filePath, int PXSize) {
	BOOL error = FT_Init_FreeType(&ftLib);  // return 0 if success
	if (error != 0) {
		return false;
	}

	error = FT_New_Face(ftLib, filePath.c_str(), 0, &ftFace);  // return 0 if success
	if (error != 0) {
		return false;
	}

	FT_Set_Pixel_Sizes(ftFace, PXSize, PXSize);
	pixelSize = PXSize;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	vbo.generate();
	vbo.bind();

	for (int i = 0; i < 128; i++) {  // create first 128 letters of unicode
		createChar(i);
	}

	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLib);
	
	vbo.upload(GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, (void*)(sizeof(glm::vec2)));
	return true;
}

bool GLFont::loadSystemFont(string name, int PXSize)
{
	char buf[512]; 
	GetWindowsDirectory(buf, 512);
	string fullPath = buf;
	fullPath += "\\Fonts\\";
	fullPath += name;

	return loadFont(fullPath, PXSize);
}

void GLFont::print(glm::mat4 projectionMatrix, string text, glm::vec3 color, int x, int y, int PXSize) {
	glBindVertexArray(vao);
	program.useProgram();
	program.setUniform("projectionMatrix", projectionMatrix);
	program.setUniform("color", glm::vec4(color, 1.0f));
	program.setUniform("gSampler", 0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int iCurX = x, iCurY = y;
	if (PXSize == -1) PXSize = pixelSize;
	float fScale = float(PXSize) / float(pixelSize);

	for (int i = 0; i < text.size(); i++) {
		if (text[i] == '\n') {
			iCurX = x;
			iCurY -= iNewLine * PXSize / pixelSize;
			continue;
		}

		int index = int(text[i]);
		iCurX += iBearingX[index] * PXSize / pixelSize;
		if (text[i] != ' ') {
			charTextures[index].bindTexture();
			glm::mat4 mModelView = glm::translate(glm::mat4(1.0f), glm::vec3(float(iCurX), float(iCurY), 0.0f));
			mModelView = glm::scale(mModelView, glm::vec3(fScale));
			program.setUniform("modelMatrix", mModelView);
			glDrawArrays(GL_TRIANGLE_STRIP, index * 4, 4);
		}

		iCurX += (iAdvX[index] - iBearingX[index]) * PXSize / pixelSize;
	}
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

int GLFont::getLength(string text, int PXSize) {
	if (PXSize == -1) PXSize = pixelSize;
	int length = 0;
	for (int i = 0; i < text.size(); i++) {
		int index = int(text[i]);
		length += iBearingX[index] * PXSize / pixelSize;
		length += (iAdvX[index] - iBearingX[index]) * PXSize / pixelSize;
	}
	return length;
}

void GLFont::releaseFont() {
	for (int i = 0; i < 28; i++) {
		charTextures[i].releaseTexture();
	}
	vbo.deleteVBO();
	glDeleteVertexArrays(1, &vao);
}