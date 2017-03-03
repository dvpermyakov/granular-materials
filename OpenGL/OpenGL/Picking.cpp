#include "Picking.h"

ShaderProgram Picking::program;

void Picking::loadShaders() {
	program.create();
	program.attachShader(Shader("Resources\\Shaders\\picking\\picking.vert", GL_VERTEX_SHADER));
	program.attachShader(Shader("Resources\\Shaders\\picking\\picking.frag", GL_FRAGMENT_SHADER));
	program.linkProgram();
}

void Picking::init() {
	fbo.generate();
	fbo.createTexture(GL_TEXTURE_2D, GL_RGB, GL_RGB, GL_FLOAT, appMain.getWidth(), appMain.getHeight());
}

void Picking::generatePickingMap(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
	fbo.bind(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	program.useProgram();
	program.setUniform("projectionMatrix", projectionMatrix);
	program.setUniform("viewMatrix", viewMatrix);
	for (int i = 0; i < models.size(); i++) {
		program.setUniform("modelMatrix", models[i]->getModelMatrix());
		program.setUniform("objectIndex", getFloatIndex(i));
		//for (int j = 0; j < models[i]->getMeshesSize(); j++) {
		//	program.setUniform("meshIndex", getFloatIndex(j));
		//	models[i]->renderMesh(j);
		//}
		models[i]->renderGeometry();
	}
	fbo.unbind();
}

void Picking::setModels(vector<AbstractModel*> models) {
	this->models = models;
}

void Picking::addModel(AbstractModel* model) {
	models.push_back(model);
}

void Picking::removeModel(AbstractModel* model) {
	std::vector<AbstractModel*>::iterator findModel = find(models.begin(), models.end(), model);
	if (findModel != models.end()) {
		models.erase(findModel);
	}
}

AbstractModel* Picking::getPickedModel(int x, int y) {
	PixelInfo pixelInfo = getPixelInfo(x, y);
	if (pixelInfo.modelIndex == 0.0f) return NULL;
	int index = getIntIndex(pixelInfo.modelIndex);
	if (index < 0) index = 0;
	if (index >= models.size()) index = models.size() - 1;
	else return models[index];
}

float Picking::getFloatIndex(int index) {
	return (index + 1) / float(models.size());
}

int Picking::getIntIndex(float index) {
	return round(index * models.size()) - 1;
}

PixelInfo Picking::getPixelInfo(int x, int y) {
	fbo.bind(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	PixelInfo pixelInfo;
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, &pixelInfo);
	fbo.unbind();
	return pixelInfo;
}