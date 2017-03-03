#include "Ground.h"

ShaderProgram Ground::program;

void Ground::loadShaders() {
	AbstractModel::loadShaders();

	program.create();
	program.attachShader(Shader("Resources\\Shaders\\ground\\ground.vert", GL_VERTEX_SHADER));
	program.attachShader(Shader("Resources\\Shaders\\ground\\ground.frag", GL_FRAGMENT_SHADER));
	program.linkProgram();
}

Ground::Ground() : vao(0), texturePrecision(0.01), highMin(0.5f), middleMin(0.2f), lowMin(0.1f), scale(glm::vec3(50, 1, 50)) {}

void Ground::load() {
	loadHeightMap(HEIGHTMAP_TEXTURE_LOCATOIN);

	highTexture.loadFromPath(HEIGH_GROUND_TEXTURE_LOCATION, true);
	highTexture.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	highTexture.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	middleTexture.loadFromPath(MIDDLE_GROUND_TEXTURE_LOCATION, true);
	middleTexture.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	middleTexture.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	lowTexture.loadFromPath(LOW_GROUND_TEXTURE_LOCATION, true);
	lowTexture.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	lowTexture.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
}

void Ground::renderGeometry() {
	glBindVertexArray(vao);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(height * width);

	int count = (height - 1) * width * 2 + height - 1;
	glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, 0);
}


ShaderProgram* Ground::getProgram() {
	return &program;
}

void Ground::render(Matricies matricies, Camera* camera, Lights lights, bool withAnimation) {
	setShaderValues(matricies, camera, lights);

	program.setUniform("highMin", highMin);
	program.setUniform("middleMin", middleMin);
	program.setUniform("lowMin", lowMin);

	program.setUniform("highTexture", 0);
	program.setUniform("middleTexture", 1);
	program.setUniform("lowTexture", 2);
	highTexture.bindTexture(0);
	middleTexture.bindTexture(1);
	lowTexture.bindTexture(2);

	renderGeometry();
}

glm::vec3 Ground::getScale() {
	return scale;
}

float Ground::getHeight(float x, float y) {
	x /= scale.x; y /= scale.z;
	x += 0.5f; y += 0.5f;
	x *= (height - 1); y *= (width - 1);
	float z = getRowHeight((int)y, (int)x) / 255.0f;
	return z * scale.y;
}

BYTE Ground::getRowHeight(int x, int y) {
	return *(dataPointer + y * bytePerPixel + x * bytePerPixel * width);
}

bool Ground::loadHeightMap(string path) {
	FIBITMAP* dib = Texture::loadData(path);
	if (!dib) return false;

	dataPointer = FreeImage_GetBits(dib);
	height = FreeImage_GetHeight(dib);
	width = FreeImage_GetWidth(dib);

	if (dataPointer == NULL || height == 0 || width == 0 || (FreeImage_GetBPP(dib) != 24 && FreeImage_GetBPP(dib) != 8)) return false;

	bytePerPixel = FreeImage_GetBPP(dib) == 24 ? 3 : 1;

	vector<vector<glm::vec3>> vertecies(height, vector<glm::vec3>(width));
	vector<vector<glm::vec2>> coordinates(height, vector<glm::vec2>(width));

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			vertecies[i][j] = glm::vec3(
				float(i) / float(height - 1) - 0.5f,                                   // x  [-0.5; 0.5]
				float(getRowHeight(j, i)) / 255.0f,                                    // y  [0;    1]
				float(j) / float(width - 1)  - 0.5f                                    // z  [-0.5; 0.5]
			);
			coordinates[i][j] = glm::vec2(
				float(i) / float(height - 1) * height * texturePrecision,  // u [0; height/texturePrecision]
				float(j) / float(width - 1) * width * texturePrecision     // v [0; widht/texturePrecision] 
			);
		}
	}

	vector<vector<glm::vec3>> normalsUp(height - 1, vector<glm::vec3>(width - 1));
	vector<vector<glm::vec3>> normalsDown(height - 1, vector<glm::vec3>(width - 1));

	for (int i = 0; i < height - 1; i++) {
		for (int j = 0; j < width - 1; j++) {
			glm::vec3 triangleDown[] = {  // 00
				vertecies[i][j],          // *0  
				vertecies[i + 1][j],      // **
				vertecies[i + 1][j + 1]   // 00
			};
			glm::vec3 triangleUp[] = {    // 00
				vertecies[i][j],          // **  
				vertecies[i + 1][j],      // 0*
				vertecies[i + 1][j + 1]   // 00
			};
			normalsDown[i][j] = glm::normalize(glm::cross(triangleDown[0] - triangleDown[1], triangleDown[1] - triangleDown[2]));
			normalsUp[i][j] = glm::normalize(glm::cross(triangleUp[0] - triangleUp[1], triangleUp[1] - triangleUp[2]));
		}
	}

	vector<vector<glm::vec3>> normals(height, vector<glm::vec3>(width));

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			glm::vec3 normal(0.0f);
			if (i > 0 && j > 0) {  // upper left
				normal += normalsDown[i - 1][j - 1];
				normal += normalsUp[i - 1][j - 1];
			}
			if (i > 0 && j < width - 1) {  // upper right
				normal += normalsDown[i - 1][j];
			}
			if (i < height - 1 && j > 0) { // bottom left
				normal += normalsUp[i][j - 1];
			}
			if (i < height - 1 && j < width - 1) {
				normal += normalsDown[i][j];
				normal += normalsUp[i][j];
			}
			normals[i][j] = glm::normalize(normal);
		}
	}

	vbo.generate();
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			vbo.addData(&vertecies[i][j], sizeof(glm::vec3));
			vbo.addData(&coordinates[i][j], sizeof(glm::vec2));
			vbo.addData(&normals[i][j], sizeof(glm::vec3));
		}
	}

	indices.generate();
	int restartIndex = height * width;
	for (int i = 0; i < height - 1; i++) {
		for (int j = 0; j < width; j++) {
			int index;
			index = i * width + j;
			indices.addData(&index, sizeof(int));
			index = (i + 1) * width + j;
			indices.addData(&index, sizeof(int));

		}
		indices.addData(&restartIndex, sizeof(int));
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	vbo.bind();
	vbo.upload();

	indices.bind(GL_ELEMENT_ARRAY_BUFFER);
	indices.upload();

	float stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

	return true;
}

glm::mat4 Ground::getModelMatrix() {
	return glm::scale(scale);
}

int Ground::getDataHeight() {
	return height;
}

int Ground::getDataWidth() {
	return width;
}

BYTE* Ground::getData() {
	return dataPointer;
}