#include "Environment.h"

ShaderProgram AbstractModel::program;
ShaderProgram AbstractModel::normalProgram;
ShaderProgram AbstractModel::silhouetteProgram;

void AbstractModel::loadShaders() {
	program.create();
	program.attachShader(Shader("Resources\\Shaders\\model\\model.vert", GL_VERTEX_SHADER));
	program.attachShader(Shader("Resources\\Shaders\\model\\model.frag", GL_FRAGMENT_SHADER));
	program.linkProgram();

	normalProgram.create();
	normalProgram.attachShader(Shader("Resources\\Shaders\\normals\\normals.vert", GL_VERTEX_SHADER));
	normalProgram.attachShader(Shader("Resources\\Shaders\\normals\\normals.geom", GL_GEOMETRY_SHADER));
	normalProgram.attachShader(Shader("Resources\\Shaders\\normals\\normals.frag", GL_FRAGMENT_SHADER));
	normalProgram.linkProgram();

	silhouetteProgram.create();
	silhouetteProgram.attachShader(Shader("Resources\\Shaders\\silhouette\\silhouette.vert", GL_VERTEX_SHADER));
	silhouetteProgram.attachShader(Shader("Resources\\Shaders\\silhouette\\silhouette.geom", GL_GEOMETRY_SHADER));
	silhouetteProgram.attachShader(Shader("Resources\\Shaders\\silhouette\\silhouette.frag", GL_FRAGMENT_SHADER));
	silhouetteProgram.linkProgram();
}
AbstractModel::AbstractModel() : position(glm::vec3(0.0f)), rotation(glm::vec3(0.0f)), scale(glm::vec3(1.0f)), name("undefined") {}
 
void AbstractModel::setModelMatrix(glm::vec3* position, glm::vec3* rotation, glm::vec3* scale) {
	if (position) this->position = *position;
	if (rotation) this->rotation = *rotation;
	if (scale)    this->scale = *scale;
}

glm::mat4 AbstractModel::getModelMatrix() {
	glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position);
	if (rotation != glm::vec3(0.0f)) {
		matrix = glm::rotate(matrix, workingRotationAngleDegree, rotation);
	}
	return glm::scale(matrix, scale);
}

glm::vec3 AbstractModel::getPosition() {
	return position;
}

glm::vec3 AbstractModel::getRotation() {
	return rotation;
}

glm::vec3 AbstractModel::getScale() {
	return scale;
}

glm::vec3 AbstractModel::getVelocity() {
	return glm::vec3(0.0f);
}

void AbstractModel::doAction() {}
void AbstractModel::contactAction() {}

string AbstractModel::getName() {
	return name;
}

void AbstractModel::renderNormals(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, glm::mat4 modelMatrix) {
	modelMatrix = modelMatrix * getModelMatrix();
	normalProgram.useProgram();
	normalProgram.setUniform("projectionMatrix", projectionMatrix);
	normalProgram.setUniform("viewMatrix", viewMatrix);
	normalProgram.setUniform("modelMatrix", modelMatrix);
	renderGeometry();
}

ShaderProgram* AbstractModel::getProgram() {
	return &program;
}

void AbstractModel::setShaderValues(Matricies matricies, Camera* camera, Lights lights) {
	glm::mat4 modelMatrix = matricies.modelMatrix * getModelMatrix();
	ShaderProgram* program = getProgram();
	program->useProgram();
	program->setUniform("projectionMatrix", matricies.projectionMatrix);
	program->setUniform("viewMatrix", matricies.viewMatrix);
	program->setUniform("modelMatrix", modelMatrix);
	if (camera != NULL) {
		program->setUniform("camera.position", camera->getPosition());
	}
	if (lights.directLight != NULL) {
		program->setUniform("lightSpaceMatrix", lights.directLight->getProjectionMatrix() * lights.directLight->getViewMatrix());
		program->setUniform("light.direction", lights.directLight->getDirection());
		program->setUniform("light.ambient", lights.directLight->getAmbientColor());
		program->setUniform("light.diffuse", lights.directLight->getDiffuseColor());
		program->setUniform("light.specular", lights.directLight->getSpecularColor());
	}
	if (lights.pointLights != NULL) {
		program->setUniform("pointLightCount", (int)lights.pointLights->size());
		for (int i = 0; i < lights.pointLights->size(); i++) {
			program->setUniform("pointLight[" + to_string(i) + "].enable", (*lights.pointLights)[i]->isEnable());
			program->setUniform("pointLight[" + to_string(i) + "].position", (*lights.pointLights)[i]->getPosition());
			program->setUniform("pointLight[" + to_string(i) + "].ambient", (*lights.pointLights)[i]->getAmbient());
			program->setUniform("pointLight[" + to_string(i) + "].diffuse", (*lights.pointLights)[i]->getDiffuse());
			program->setUniform("pointLight[" + to_string(i) + "].constant", (*lights.pointLights)[i]->getConstant());
			program->setUniform("pointLight[" + to_string(i) + "].linear", (*lights.pointLights)[i]->getLinear());
			program->setUniform("pointLight[" + to_string(i) + "].quadratic", (*lights.pointLights)[i]->getQuadratic());
			program->setUniform("pointLight[" + to_string(i) + "].farPlane", (*lights.pointLights)[i]->getFarPlane());
			program->setUniform("pointLight[" + to_string(i) + "].nearPlane", (*lights.pointLights)[i]->getNearPlane());
		}
	}
	if (lights.shadowMap != NULL) {
		lights.shadowMap->bindTexture(10);
		program->setUniform("shadowMap", 10);
		program->setUniform("withShadow", true);
	}
	else {
		program->setUniform("withShadow", false);
	}
	if (lights.cubeShadowMaps != NULL) {
		for (int i = 0; i < lights.cubeShadowMaps->size(); i++) {
			Texture* cubeTexture = (*lights.cubeShadowMaps)[i];
			cubeTexture->bindTexture(i + 11);
			program->setUniform("cubeShadowMap[" + to_string(i) + "]", i + 11);
		}
	}
}

void AbstractModel::release() {}

// EnvironmentModel

EnvironmentModel::EnvironmentModel() : AbstractModel(), model(NULL), path("") {}

void EnvironmentModel::load(string path, bool generateAdjacencies) {
	if (model != NULL && model->isLoaded()) {
		model->release();
	}
	model = new Model();
	model->loadModelFromFile(path, generateAdjacencies);
	model->generateVao();
	if (generateAdjacencies) model->generateAdjacencyVao();
	name = model->getPath();
	this->path = path;
}

Model* EnvironmentModel::getModel() {
	return model;
}

int EnvironmentModel::getMeshesSize() {
	return getModel()->getMeshesSize();
}

string EnvironmentModel::getPath() {
	return path;
}

void EnvironmentModel::renderMesh(int index) {
	Model* model = getModel();
	model->bindVao();
	model->renderMesh(index);
}

void EnvironmentModel::renderGeometry() {
	Model* model = getModel();
	model->bindVao();
	model->renderGeometry();
}

void EnvironmentModel::renderSilhouetteGeometry() {
	Model* model = getModel();
	model->bindAdjacencyVao();
	model->renderAdjacencyGeometry();
}

void EnvironmentModel::renderSilhouette(Matricies matricies, Lights lights) {
	glm::mat4 modelMatrix = matricies.modelMatrix * getModelMatrix();
	silhouetteProgram.useProgram();
	silhouetteProgram.setUniform("projectionMatrix", matricies.projectionMatrix);
	silhouetteProgram.setUniform("viewMatrix", matricies.viewMatrix);
	silhouetteProgram.setUniform("modelMatrix", modelMatrix);
	silhouetteProgram.setUniform("lightPosition", lights.directLight->getPosition());

	glLineWidth(5.0f);
	//glDepthFunc(GL_LEQUAL);
	renderSilhouetteGeometry();
}

void EnvironmentModel::render(Matricies matricies, Camera* camera, Lights lights, bool onlyAmbient) {
	setShaderValues(matricies, camera, lights);
	program.setUniform("onlyAmbient", onlyAmbient);
	Model* model = getModel();
	model->bindVao();
	for (int i = 0; i < model->getMeshesSize(); i++) {
		Material* material = model->getMaterial(i);
		program.setUniform("material.shininess", material->getShininess());
		if (material->getDiffuseTexture().wasCreated()) {
			program.setUniform("material.diffuse", 0);
			material->getDiffuseTexture().bindTexture(0);
		}
		map<string, Bone*>* bones = model->getBones();
		bool withAnimation = false;  // this is hardcode cos animation is not used
		if (bones->size() == 0 || !withAnimation) {
			program.setUniform("withBones", false);
		}
		else {
			program.setUniform("withBones", true);
			int index = 0;
			for (map<string, Bone*>::iterator iterator = bones->begin(); iterator != bones->end(); iterator++, index++) {
				string locationName = "boneMatricies[" + to_string(index) + "]";
				program.setUniform(locationName, iterator->second->getTransformation());
			}
		}
		model->renderMesh(i);
	}
}

void EnvironmentModel::release() {
	if (model != NULL) model->release();
}

// Lamp

Model Lamp::lampModel;

void Lamp::loadModel() {
	lampModel.loadModelFromFile(LAMP_MODEL_LOCATION);
	lampModel.generateVao();
}

Lamp::Lamp(PointLight light) : EnvironmentModel(), light(light), shadowLength(2048) {
	shadowMap.init(GL_TEXTURE_CUBE_MAP, shadowLength, shadowLength);
	name = "Lamp";
}

Model* Lamp::getModel() {
	return &lampModel;
}

PointLight* Lamp::getLight() {
	return &light;
}

glm::mat4 Lamp::getModelMatrix() {
	return glm::translate(light.getPosition());
}

ShadowMap* Lamp::getShadowMap() {
	return &shadowMap;
}

void Lamp::doAction() {
	light.switchLight();
}