#include "ShaderProgram.h"
#include "GLControl.h"
#include "WindowApp.h"
#include "Sky.h"
#include "Camera.h"
#include "Text.h"
#include "Buffer.h"
#include "Ground.h"
#include "Bullet.h"
#include "Audio.h"
#include "Environment.h"
#include "Picking.h"
#include "Screen.h"
#include "Resources.h"
#include "UserInterface.h"
#include "Granule.h"
#include "Physics.h"
#include "ParticleSystem.h"

#include <fstream>

using namespace std;

GranuleModel model;
PhysicEngine pEngine;
SkyBox sky;
Ground ground;
Sun sun;
Camera camera;
GLFont font;
vector<Bullet*> bullets;
vector<Lamp*> lamps;
Picking picking;
vector<ParticleSystem*> particleSystems;

bool renderNormals = false;
bool updateSun = false;
bool isLoaded = false;
bool granular = false;
bool renderDefug = false;
bool wireFrame = false;
bool stencilShadow = true;

SplashScreen splashScreen;

void InitScene(LPVOID lpParam) {
	GLControl* glControl = (GLControl*)lpParam;
	GLFont::loadShaders();

	splashScreen.setFont("arial.ttf", 32);
	splashScreen.setGLControl(glControl);

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
}

void loadScene(GLControl& glControl) {
	splashScreen.render("shaders");
	SkyBox::loadShaders();
	Sun::loadShaders();
	EnvironmentModel::loadShaders();
	ShadowMap::loadShaders();
	Picking::loadShaders();
	Ground::loadShaders();
	Granule::loadShaders();
	DebugDraw::loadShaders();
	ParticleSystem::loadShaders();
	VolumeShadow::loadShaders();

	splashScreen.render("physics");
	DebugDraw::setControl(&glControl);
	DebugDraw::setCamera(&camera);
	DebugDraw::setFont(&font);
	pEngine.load();

	splashScreen.render("interface");
	ATBInterface::init(&glControl);

	splashScreen.render("sound");
	SoundEngine::init();

	splashScreen.render("bullet model");
	Bullet::loadModel();

	splashScreen.render("sky model");
	sky.load();

	splashScreen.render("ground model");
	ground.load();
	//pEngine.addTerrain(&ground);

	splashScreen.render("main model"); 
	//std::ifstream ifs(MAIN_MODEL_WITH_GRANULES_LOCATION);
	//boost::archive::text_iarchive ia(ifs);
	//ia >> model;
	//model.loadGranules();
	model = GranuleModel();
	model.load(MAIN_MODEL_LOCATION);
	model.setModelMatrix(&glm::vec3(0, ground.getHeight(0, 0), 0));
	vector<Granule*> granules;
	granules.push_back(new Granule("granule1", glm::vec3(1, 1, 1), glm::vec3(10, 5, 10), CLAY, 32.0f, 1.0f));
	granules.push_back(new Granule("granule2", glm::vec3(1, 1, 1), glm::vec3(15, 5, 10), MOSS, 0.0f, 0.3f));
	granules.push_back(new Granule("granule3", glm::vec3(1, 1, 1), glm::vec3(20, 5, 10), PEBBLE, 16.0f, 0.1f));
	granules.push_back(new Granule("granule4", glm::vec3(1, 1, 1), glm::vec3(25, 5, 10), ROCK, 128.0f, 1.0f));
	granules.push_back(new Granule("granule5", glm::vec3(1, 1, 1), glm::vec3(30, 5, 10), SAND, 0.0f, 1.0f));
	model.setGranules(granules);
	//pEngine.addRigidBody(&model, 100.0f, COMMON);

	splashScreen.render("sun model");
	sun.load();

	splashScreen.render("lights");
	Lamp::loadModel();
	lamps.push_back(new Lamp(PointLight(glm::vec3(-7, ground.getHeight(-7, 0), 0), glm::vec3(0.7f), glm::vec3(1.0f), DISTANCE_65)));
	lamps.push_back(new Lamp(PointLight(glm::vec3(7, ground.getHeight(7, 0), 0), glm::vec3(0.7f), glm::vec3(1.0f), DISTANCE_65)));

	splashScreen.render("granules");
	for (int i = 0; i < model.getGranules()->size(); i++) {
		pEngine.addRigidBody((*model.getGranules())[i].granule, 0.0f, BIG_GRANULE);
	}

	splashScreen.render("picking system");
	picking.init();
	picking.addModel(&model);
	for (int i = 0; i < lamps.size(); i++) {
		picking.addModel(lamps[i]);
	}
	for (int i = 0; i < model.getGranules()->size(); i++) {
		picking.addModel((*model.getGranules())[i].granule);
	}

	splashScreen.render("fonts");
	font.loadSystemFont("arial.ttf", 32);

	camera.setCursor();
	isLoaded = true;
}

void showInformation(GLControl& glControl) {
	int granules = 0;
	for (int i = 0; i < model.getGranules()->size(); i++) {
		granules += (*model.getGranules())[i].vectors.size();
	}
	font.print(*glControl.getOrthoMatrix(), "FPS = " + to_string(glControl.getFps()), glm::vec3(1, 1, 1), 10, glControl.getViewPortHeight() - 30);
	font.print(*glControl.getOrthoMatrix(), "Granules = " + to_string(granules), glm::vec3(1, 1, 1), 10, glControl.getViewPortHeight() - 60);
	font.print(*glControl.getOrthoMatrix(), "Triangles = " + to_string(model.getTriangelMap()->size()), glm::vec3(1, 1, 1), 10, glControl.getViewPortHeight() - 90);
}

void updateSunShadows(ShadowMap* shadowMap, DirectLight* sunLight) {
	if (shadowMap->getTexture() == NULL || updateSun) {
		shadowMap->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadowMap->setViewProjectionMatrices(sunLight);
		shadowMap->setModelMatrix(ground.getModelMatrix());
		ground.renderGeometry();
		if (granular) {
			shadowMap->bind(true);
			shadowMap->setViewProjectionMatrices(sunLight, true);
			shadowMap->setModelMatrix(glm::mat4(1.0f), true);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			model.renderGranularGeometry();
			glCullFace(GL_FRONT);
			glDisable(GL_CULL_FACE);
		}
		else {
			shadowMap->setModelMatrix(model.getModelMatrix());
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			model.renderGeometry();
			glCullFace(GL_FRONT);
			glDisable(GL_CULL_FACE);
		}
		shadowMap->anchorTexture();
		shadowMap->unbind();
	}
}

void updateLampShadow(ShadowMap* shadowMapCube, PointLight* lampLight) {
	if (shadowMapCube->getTexture() == NULL) {
		shadowMapCube->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadowMapCube->setViewProjectionMatrices(lampLight);
		shadowMapCube->setModelMatrix(ground.getModelMatrix());
		ground.renderGeometry();
		shadowMapCube->setModelMatrix(model.getModelMatrix());
		model.renderGeometry();
		shadowMapCube->anchorTexture();
		shadowMapCube->unbind();
	}
}

void updateStencilShadow(GLControl& glControl, DirectLight* sunLight) {
	if (stencilShadow) {
		VolumeShadow::zBufferProgram.useProgram();
		VolumeShadow::zBufferProgram.setUniform("projectionMatrix", *glControl.getProjectionMatrix());
		VolumeShadow::zBufferProgram.setUniform("viewMatrix", camera.getViewMatrix());
		VolumeShadow::zBufferProgram.setUniform("modelMatrix", model.getModelMatrix());
		model.renderGeometry();

		glEnable(GL_STENCIL_TEST);

		//glDrawBuffer(GL_NONE);
		glDepthMask(GL_FALSE);

		glStencilFunc(GL_ALWAYS, 0x0, 0xFF);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		VolumeShadow::stencilProgram.useProgram();
		VolumeShadow::stencilProgram.setUniform("projectionMatrix", *glControl.getProjectionMatrix());
		VolumeShadow::stencilProgram.setUniform("viewMatrix", camera.getViewMatrix());
		VolumeShadow::stencilProgram.setUniform("modelMatrix", model.getModelMatrix());
		VolumeShadow::stencilProgram.setUniform("lightPosition", sunLight->getPosition());
		model.renderSilhouetteGeometry();

		glStencilFunc(GL_EQUAL, 0x0, 0xFF);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
		//glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_KEEP);

		//glDrawBuffer(GL_BACK);
		glDepthMask(GL_TRUE);
	}
}

void updateScene(DirectLight* sunLight) {
	camera.update();

	if (updateSun) sunLight->update();

	for (int i = 0; i < bullets.size(); i++) {
		if (!bullets[i]->isAlive()) {
			bullets.erase(bullets.begin() + i);
		}
	}

	pEngine.update();
	ObjectsForRendering* objects = pEngine.getObjectsForRendering();
	for (int i = 0; i < objects->particleSystems.size(); i++) {
		particleSystems.push_back(new ParticleSystem(*objects->particleSystems[i]));
	}
	pEngine.resetObjectsForRendering();
}

void updateChoosenModel(GLControl& glControl) {
	picking.generatePickingMap(*glControl.getProjectionMatrix(), camera.getViewMatrix());

	pair<int, int> cursorPosition = camera.getCursorPosition();
	AbstractModel* chosenModel = picking.getPickedModel(cursorPosition.first, cursorPosition.second);
	if (chosenModel != NULL) {
		int nameLength = font.getLength(chosenModel->getName());
		int width = (glControl.getViewPortWidth() - nameLength) / 2;
		int height = glControl.getViewPortHeight() - 30;
		font.print(*glControl.getOrthoMatrix(), chosenModel->getName(), glm::vec3(1, 1, 1), width, height);
	}

	if (Key::wasOnePressed(int('E'))) if (chosenModel) chosenModel->doAction();
}

void updateKeys(DirectLight* sunLight) {
	if (Key::wasPressed(VK_ESCAPE)) PostQuitMessage(0);

	if (Key::wasOnePressed(int('Z'))) {
		renderDefug = !renderDefug;
		pEngine.setRenderDebug(renderDefug);
	}
	if (Key::wasOnePressed(int('B'))) updateSun = !updateSun;
	if (Key::wasOnePressed(int('N'))) renderNormals = !renderNormals;
	if (Key::wasOnePressed(int('G'))) granular = !granular;
	if (Key::wasOnePressed(int('K'))) wireFrame = !wireFrame;
	if (Key::wasOnePressed(int('I'))) stencilShadow = !stencilShadow;
	if (Key::wasOnePressed(VK_OEM_PLUS)) sunLight->moveUp();
	if (Key::wasOnePressed(VK_OEM_MINUS)) sunLight->moveDown();

	if (Key::wasOnePressed(VK_SPACE)) {
		Bullet* bullet = new Bullet(camera);
		bullets.push_back(bullet);
		pEngine.addRigidBody(bullet, 1.0f, BULLET);
		sound.play2D(SOUND_SHOT);
	}

	if (Key::wasPressed(VK_CONTROL) && Key::wasOnePressed(int('M'))) {
		model.setGranulesExemplars();
	}

	if (Key::wasPressed(VK_CONTROL) && Key::wasOnePressed(int('Q'))) {
		string path = appMain.saveFileDialog();
		if (path.size() > 0) {
			ofstream ofs(path);
			boost::archive::text_oarchive oa(ofs);
			oa << model;
		}
	}

	if (Key::wasPressed(VK_CONTROL) && Key::wasOnePressed(int('O'))) {
		string path = appMain.openFileDialog();
		if (path.size() > 0) {
			GranuleModel newModel;
			bool load = false;
			try {
				load = true;
				std::ifstream ifs(path);
				boost::archive::text_iarchive ia(ifs);
				ia >> newModel;
			}
			catch (...) {
				load = false;
			}
			if (load) {
				newModel.loadGranules();
				for (int i = 0; i < (*model.getGranules()).size(); i++) {
					picking.removeModel((*model.getGranules())[i].granule);
				}
				ATBInterface::clearMap();
				pEngine.removeBigGranules();
				model = newModel;
				for (int i = 0; i < (*model.getGranules()).size(); i++) {
					picking.addModel((*model.getGranules())[i].granule);
				}
				for (int i = 0; i < model.getGranules()->size(); i++) {
					pEngine.addRigidBody((*model.getGranules())[i].granule, 0.0f, BIG_GRANULE);
				}
			}
			else {
				MessageBox(appMain.hWnd, "Couldn't load granular model", "Error Importing Granular Model", MB_ICONERROR);
			}
		}
	}
}

void RenderScene(LPVOID lpParam) {
	GLControl* glControl = (GLControl*)lpParam;

	if (!isLoaded) {
		loadScene(*glControl);
		return;
	}

	if (model.getStatus() == WITH_GRANULE_TYPES) {
		model.setGranulesExemplars();
	}
	if (model.getStatus() == WITH_EXEMPLARS) {
		pEngine.addGranularBody(&model);
	}

	DirectLight* sunLight = sun.getLight();

	vector<PointLight*> lampLights;
	for (int i = 0; i < lamps.size(); i++) {
		lampLights.push_back(lamps[i]->getLight());
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_STENCIL_TEST);
	//updateStencilShadow(*glControl, sunLight);

	ShadowMap* shadowMap = sun.getShadowMap();
	updateSunShadows(shadowMap, sunLight);

	vector<Texture*> pointLightShadowTextures;
	for (int i = 0; i < lamps.size(); i++) {
		ShadowMap* shadowMapCube = lamps[i]->getShadowMap();
		//updateLampShadow(shadowMapCube, lamps[i]->getLight());
		pointLightShadowTextures.push_back(shadowMapCube->getTexture());
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (renderNormals) {
		model.renderNormals(*glControl->getProjectionMatrix(), camera.getViewMatrix(), glm::translate(glm::vec3(0, ground.getHeight(0, 0), 0)));
		ground.renderNormals(*glControl->getProjectionMatrix(), camera.getViewMatrix(), glm::mat4(1.0f));
	}
	sky.render(*glControl->getProjectionMatrix(), glm::translate(camera.getViewMatrix(), camera.getPosition()), *sunLight);
	sun.render(*glControl->getProjectionMatrix(), glm::translate(camera.getViewMatrix(), camera.getPosition())); 
	Matricies mats = { *glControl->getProjectionMatrix(), camera.getViewMatrix(), glm::mat4(1.0f) };
	Lights lights = { sunLight, &lampLights, shadowMap->getTexture(), NULL };

	ground.render(mats, &camera, lights);
	if (granular) {
		model.renderGranular(mats, &camera, lights);
		if (wireFrame) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			model.render(mats, &camera, lights);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	else model.render(mats, &camera, lights);
	//model.renderSilhouette(mats, lights);

	for (int i = 0; i < model.getGranules()->size(); i++) {
		(*model.getGranules())[i].granule->render(mats, &camera, lights);
	}

	lights.cubeShadowMaps = NULL;
	for (int i = 0; i < lamps.size(); i++) {
		//lamps[i]->render(mats, &camera, lights);
	}

	lights.shadowMap = NULL;
	for (int i = 0; i < bullets.size(); i++) {
		bullets[i]->render(mats, &camera, lights);
	}

	for (int i = 0; i < particleSystems.size(); i++) {
		particleSystems[i]->update();
		if (particleSystems[i]->isAlive()) {
			particleSystems[i]->render(mats.projectionMatrix, &camera);
		}
	}

	/*if (stencilShadow && !granular) {
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		model.render(mats, &camera, lights, true);
		glDisable(GL_BLEND);
	}*/

	updateScene(sunLight);
	updateChoosenModel(*glControl);
	updateKeys(sunLight);

	bool findRenderSettings = false;
	for (int i = 0; i < model.getGranules()->size(); i++) {
		if ((*model.getGranules())[i].granule->isRenderingSettings()) {
			camera.setActive(false);
			appMain.hideCursor = false;
			ATBInterface::render((*model.getGranules())[i].granule);
			findRenderSettings = true;
			break;
		}
	}
	if (model.isRenderingSettings()) {
		camera.setActive(false);
		appMain.hideCursor = false;
		ATBInterface::render(&model);
		findRenderSettings = true;
	}
	if (!findRenderSettings) {
		camera.setCursor();
		camera.setActive(true);
		appMain.hideCursor = true;
		ATBInterface::removeAllBars();
	}

	if (Key::wasPressed(VK_RETURN)) {
		for (int i = 0; i < model.getGranules()->size(); i++) {
			(*model.getGranules())[i].granule->setRenderingSettings(false);
		}
		model.setRenderingSettings(false);
	}

	showInformation(*glControl);

	glControl->SwapBuffersM();
}

void ReleaseScene(LPVOID lpParam) {}