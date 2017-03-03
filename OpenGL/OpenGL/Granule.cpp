#include "Granule.h"


GranuleGeometry::GranuleGeometry() : scale(Octahedron::radius) {}

GranuleGeometry::GranuleGeometry(GranuleGeometrySettings settings) : settings(settings), scale(Octahedron::radius) {
	generateBuffers();
	generate();
}

void GranuleGeometry::generateBuffers() {
	vbo.generate();
	indicies.generate();
}

void GranuleGeometry::generate() {
	GeometryInfo gi;
	gi.points = vector<glm::vec3>(Octahedron::points, Octahedron::points + Octahedron::vertexCount);
	gi.indicies = vector<UINT>(Octahedron::indicies, Octahedron::indicies + Octahedron::indexCount);
	for (int i = 0; i < settings.fragCount; i++) {
		gi = trianlgeFragmentation(gi.points, gi.indicies);
	}
	float radius = Octahedron::radius;
	for (int i = 0; i < gi.points.size(); i++) {
		float randValue = getFloatRandom() * settings.sharpDegree;
		gi.points[i] *= (radius - randValue) / glm::length(gi.points[i]);
	}
	for (int k = 0; k < settings.smoothCount; k++)
		for (int i = 0; i < gi.indicies.size(); i += 3) {
			UINT i0, i1, i2;
			i0 = gi.indicies[i]; i1 = gi.indicies[i + 1]; i2 = gi.indicies[i + 2];
			float delta0 = (glm::length(gi.points[i1]) + glm::length(gi.points[i2]) - 2.0f * glm::length(gi.points[i0])) / 2.0f;
			float delta1 = (glm::length(gi.points[i0]) + glm::length(gi.points[i2]) - 2.0f * glm::length(gi.points[i1])) / 2.0f;
			float delta2 = (glm::length(gi.points[i0]) + glm::length(gi.points[i1]) - 2.0f * glm::length(gi.points[i2])) / 2.0f;
			gi.points[i0] *= 1.0f + delta0;
			gi.points[i1] *= 1.0f + delta1;
			gi.points[i2] *= 1.0f + delta2;
		}
	scale = 0.0f;
	for (int i = 0; i < gi.points.size(); i++) {
		if (glm::length(gi.points[i]) > scale) scale = glm::length(gi.points[i]);
	}

	vbo.addData(&gi.points[0], sizeof(glm::vec3) * gi.points.size());
	indicies.addData(&gi.indicies[0], sizeof(UINT) * gi.indicies.size());

	vector<glm::vec3> tangents;
	for (int i = 0; i < gi.points.size(); i++) {
		tangents.push_back(glm::vec3(0.0f));
	}
	for (int i = 0; i < gi.indicies.size(); i += 3) {
		UINT i0, i1, i2;
		i0 = gi.indicies[i]; i1 = gi.indicies[i + 1]; i2 = gi.indicies[i + 2];
		glm::vec3 tangent = getTangentVector(gi.points[i0], gi.points[i1], gi.points[i2]);
		tangents[i0] += tangent;
		tangents[i1] += tangent;
		tangents[i2] += tangent;
	}
	for (int i = 0; i < gi.points.size(); i++) {
		tangents[i] = glm::normalize(tangents[i]);
	}
	vbo.addData(&tangents[0], sizeof(glm::vec3) * gi.points.size());

	indexCount = gi.indicies.size();
	pointCount = gi.points.size();
}

GLBuffers GranuleGeometry::createVao(vector<glm::mat4>* modelMatricies, vector<int>* triangleIndecies) {
	VertexBufferObject* vbo = new VertexBufferObject(this->vbo);

	for (int i = 0; i < triangleIndecies->size(); i++) {
		vbo->addData(&(*triangleIndecies)[i], sizeof(int));
	}

	for (int i = 0; i < modelMatricies->size(); i++) {
		vbo->addData(&(*modelMatricies)[i], sizeof(glm::mat4));
	}

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	vbo->bind();
	vbo->upload(GL_DYNAMIC_DRAW, false);
	indicies.bind(GL_ELEMENT_ARRAY_BUFFER);
	indicies.upload(GL_STATIC_DRAW, false);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(glm::vec3) * pointCount));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_INT, GL_FALSE, sizeof(int), (GLvoid*)(this->vbo.getSize()));

	int sizeBeforeInstanceMatricies = this->vbo.getSize() + (*triangleIndecies).size() * sizeof(int);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeBeforeInstanceMatricies + 0 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeBeforeInstanceMatricies + 1 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeBeforeInstanceMatricies + 2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeBeforeInstanceMatricies + 3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	return{ vao, vbo, sizeBeforeInstanceMatricies };
}

VertexBufferObject* GranuleGeometry::getVBO() {
	return &vbo;
}

VertexBufferObject* GranuleGeometry::getIndicies() {
	return &indicies;
}

GranuleGeometrySettings* GranuleGeometry::getGranuleGeometrySettings() {
	return &settings;
}

float GranuleGeometry::getScale() {
	return scale;
}

void GranuleGeometry::render(int amount) {
	glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, amount);
}

ShaderProgram Granule::program;

void Granule::loadShaders() {
	AbstractModel::loadShaders();

	program.create();
	program.attachShader(Shader("Resources\\Shaders\\granule\\granule.vert", GL_VERTEX_SHADER));
	program.attachShader(Shader("Resources\\Shaders\\granule\\granule.frag", GL_FRAGMENT_SHADER));
	program.linkProgram();
}

Granule::Granule() : AbstractModel(), vao(0), renderSettings(false), probability(1.0f) {}

Granule::Granule(string name, glm::vec3 color, glm::vec3 position, TextureType type, float shininess, float probability) : color(color), type(type), shininess(shininess), probability(probability), geometry(GranuleGeometry({ 1, 1, 0.8f })), vao(0), renderSettings(false) {
	this->position = position;
	this->scale = glm::vec3(2.0f);
	this->name = name;
	generateGeometry();
	setTexture();
	vector<bool> data = { true };
	triangleTexture = getTriangleTexture(data);
}

void Granule::renderGeometry() {
	glBindVertexArray(vao);
	geometry.render(1);
}

void Granule::renderGeometry(int amount) {
	geometry.render(amount);
}

void Granule::render(Matricies matricies, Camera* camera, Lights lights, bool onlyAmbient) {
	renderAmount(matricies, camera, lights, vao, 1, &triangleTexture);
}

void Granule::renderAmount(Matricies matricies, Camera* camera, Lights lights, GLuint vao, int amount, Texture* triangleTexture) {
	glm::mat4 modelMatrix = matricies.modelMatrix;
	program.useProgram();
	program.setUniform("projectionMatrix", matricies.projectionMatrix);
	program.setUniform("viewMatrix", matricies.viewMatrix);
	program.setUniform("modelMatrix", modelMatrix);

	program.setUniform("material.color", color);
	program.setUniform("material.diffuse", 0);
	program.setUniform("material.normal", 1);
	program.setUniform("material.shininess", shininess);

	if (lights.directLight != NULL) {
		program.setUniform("lightSpaceMatrix", lights.directLight->getProjectionMatrix() * lights.directLight->getViewMatrix());
		program.setUniform("light.direction", lights.directLight->getDirection());
		program.setUniform("light.ambient", lights.directLight->getAmbientColor());
		program.setUniform("light.diffuse", lights.directLight->getDiffuseColor());
		program.setUniform("light.specular", lights.directLight->getSpecularColor());
	}
	if (camera != NULL) {
		program.setUniform("camera.position", camera->getPosition());
	}
	if (lights.shadowMap != NULL) {
		lights.shadowMap->bindTexture(2);
		program.setUniform("shadowMap", 2);
	}

	if (triangleTexture != NULL) {
		program.setUniform("useTriangleEnable", true);
		program.setUniform("triangleEnable", 3);
		triangleTexture->bindTexture(3);
	}
	else {
		program.setUniform("useTriangleEnable", false);
	}

	glBindVertexArray(vao);
	colorTexture.bindTexture(0);
	normalTexture.bindTexture(1);
	renderGeometry(amount);
}

VertexBufferObject* Granule::getVBO() {
	return geometry.getVBO();
}

VertexBufferObject* Granule::getIndicies() {
	return geometry.getIndicies();
}

glm::mat4 Granule::getModelMatrix() {
	return glm::scale(glm::translate(glm::mat4(1.0f), position), scale);
}

glm::vec3 Granule::getScale() {
	return scale * geometry.getScale();
}

void Granule::doAction() {
	renderSettings = true;
}

void Granule::setRenderingSettings(bool renderSettings) {
	this->renderSettings = renderSettings;
}

bool Granule::isRenderingSettings() {
	return renderSettings;
}

glm::vec3* Granule::getColorRef() {
	return &color;
}

void Granule::setTexture() {
	string colorPath;
	switch (type) {
	case CLAY:
		colorPath = CLAY_COLOR_GRANULE_TEXTURE_LOCATION;
		break;
	case MOSS:
		colorPath = MOSS_COLOR_GRANULE_TEXTURE_LOCATION;
		break;
	case PEBBLE:
		colorPath = PEBBLE_COLOR_GRANULE_TEXTURE_LOCATION;
		break;
	case ROCK:
		colorPath = ROCK_COLOR_GRANULE_TEXTURE_LOCATION;
		break;
	case SAND:
		colorPath = SAND_COLOR_GRANULE_TEXTURE_LOCATION;
		break;
	}

	colorTexture.releaseTexture();
	colorTexture.generate(GL_TEXTURE_CUBE_MAP);
	colorTexture.addTexture(colorPath, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	colorTexture.addTexture(colorPath, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	colorTexture.addTexture(colorPath, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	colorTexture.addTexture(colorPath, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	colorTexture.addTexture(colorPath, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	colorTexture.addTexture(colorPath, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	glGenerateMipmap(colorTexture.getTarget());
	colorTexture.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	colorTexture.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	colorTexture.setSamplerParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	colorTexture.setSamplerParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	colorTexture.setSamplerParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	/*normalTexture.releaseTexture();
	normalTexture.generate(GL_TEXTURE_CUBE_MAP);
	normalTexture.addTexture(NORMAL_GRANULE_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	normalTexture.addTexture(NORMAL_GRANULE_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	normalTexture.addTexture(NORMAL_GRANULE_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	normalTexture.addTexture(NORMAL_GRANULE_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	normalTexture.addTexture(NORMAL_GRANULE_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	normalTexture.addTexture(NORMAL_GRANULE_TEXTURE_LOCATION, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	normalTexture.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	normalTexture.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	normalTexture.setSamplerParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	normalTexture.setSamplerParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
}

Texture Granule::getTriangleTexture(vector<bool> data) {
	Texture texture = Texture();
	vector<glm::vec2> dataFloat;
	for (int i = 0; i < data.size(); i++) {
		float value = 0.0;
		if (data[i]) value = 1.0f;
		dataFloat.push_back(glm::vec2(value));
	}
	texture.loadFromData1D(&dataFloat[0], dataFloat.size(), GL_RG, GL_RG, GL_FLOAT);
	return texture;
}

float* Granule::getShininessRef() {
	return &shininess;
}

TextureType* Granule::getTextureTypeRef() {
	return &type;
}

float* Granule::getProbabilityRef() {
	return &probability;
}

GranuleGeometrySettings* Granule::getGranuleGeometrySettings() {
	return geometry.getGranuleGeometrySettings();
}

void Granule::generateGeometry(bool toGenerate) {
	if (toGenerate) {
		geometry = GranuleGeometry(*geometry.getGranuleGeometrySettings());
	}
	else {
		geometry.generateBuffers();
	}
	vector<glm::mat4> matricies({ getModelMatrix() });
	vector<int> triangleIndecies({ 0 });  // always only one triangle
	vao = geometry.createVao(&matricies, &triangleIndecies).vao;
}

GLBuffers Granule::createVao(vector<glm::mat4>* modelMatricies, vector<int>* triangleIndecies) {
	return geometry.createVao(modelMatricies, triangleIndecies);
}

GranuleModel::GranuleModel() : EnvironmentModel(), scaleIndex(0.004f), status(EMPTY), volumeLength(2.0f), minLength(3.0f), maxLength(10.0f), maxTriangles(0), renderSettings(false) {
	this->name = "Granular Model";
}

void GranuleModel::doAction() {
	renderSettings = true;
}

string GranuleModel::getName() {
	return "GranularModel";
}

void GranuleModel::setGranules(vector<Granule*> granuleTypes) {
	granules.clear();
	this->granuleTypes = granuleTypes;
	for (int i = 0; i < granuleTypes.size(); i++) {
		granules.push_back({ vector<GranuleVectors>(), 0, NULL, 0, granuleTypes[i] });
	}
	status = WITH_GRANULE_TYPES;
	setTriangles();
}

void GranuleModel::loadGranules() {
	if (getModel() == NULL && getPath() != "") {
		load(getPath());
	}
	for (int i = 0; i < granules.size(); i++) {
		granules[i].granule->generateGeometry(false);
		granules[i].granule->setTexture();
		granuleTypes.push_back(granules[i].granule);
	}
	setTriangles();
	status = COMPRESSED;
}

void GranuleModel::setTriangles() {
	vector<glm::vec3>* positions = getModel()->getVertecies();
	vector<glm::vec3>* normals = getModel()->getNormals();
	vector<UINT>* indicies = getModel()->getIndicies();

	for (int i = 0; i < indicies->size(); i += 3) {
		UINT i0 = (*indicies)[i], i1 = (*indicies)[i + 1], i2 = (*indicies)[i + 2];
		glm::vec3 position0 = (*positions)[i0], position1 = (*positions)[i1], position2 = (*positions)[i2];
		glm::vec3 normal = glm::normalize(getTriangleCenter({ (*normals)[i0], (*normals)[i1], (*normals)[i2] }));
		triangles.push_back({ vector<glm::vec3>({ position0, position1, position2 }), normal, minLength, maxLength });
	}
	generateVaos();
}

void GranuleModel::setGranulesExemplars() {
	for (int i = 0; i < granules.size(); i++) {
		granules[i].vectors.clear();
	}
	triangleMap.clear();

	appMain.createProgressBar(min(maxTriangles, (int)triangles.size()));
	for (int i = 0; i < triangles.size(); i++) {
		if (i >= maxTriangles) break;
		addGranulesInTriangle(i, { triangles[i].positions[0], triangles[i].positions[1], triangles[i].positions[2] }, triangles[i].normal, volumeLength, maxLength);
		appMain.stepProgressBar();
	}
	appMain.removeProgressBar();
	generateVaos();
	status = WITH_EXEMPLARS;
}

void GranuleModel::addGranulesInTriangle(int triangleIndex, TrianglePoints points, glm::vec3 normal, float minLength, float maxLength) {
	triangleMap[triangleIndex] = new vector<GranuleVectors*>();

	float factor = 1.0f;
	if (getTriangleSquare(points) < (getSphereSquare(getAvgScale()) * ((minLength + maxLength) / 2.0f / volumeLength))) {
		factor = getTriangleSquare(points) / (getSphereSquare(getAvgScale()) * ((minLength + maxLength) / 2.0f / volumeLength));
	}
	vector<float> scales = getScales();
	for (int i = 0; i < scales.size(); i++) {
		scales[i] *= factor;
	}
	vector<float> probabilities;
	for (int i = 0; i < granules.size(); i++) {
		probabilities.push_back(*granules[i].granule->getProbabilityRef());
	}
	vector<RandomPoints> granulePositions = getRandomPointsInTriangle(scales, probabilities, points, -normal, minLength, maxLength, getAvgScale() * factor);
	for (int j = 0; j < granulePositions.size(); j++) {
		addGranule(granulePositions[j].positions, glm::vec3(getScale(granulePositions[j].granuleIndex)) * factor, granulePositions[j].granuleIndex, triangleIndex);
	}
}

void GranuleModel::addGranule(glm::vec3 position, glm::vec3 scale, int granuleIndex, int triangleIndex) {
	//granules[granuleIndex].vectors.push_back({ position, getRandomVector(), scale, granuleIndex, granules[granuleIndex].vectors.size(), triangleIndex });
	granules[granuleIndex].vectors.push_back({ position, getRandomVector(), scale, granuleIndex, granules[granuleIndex].vectors.size() });
	triangleMap[triangleIndex]->push_back(new GranuleVectors(granules[granuleIndex].vectors[granules[granuleIndex].vectors.size() - 1]));
}


glm::mat4 GranuleModel::createModelMatrix(int granuleIndex, int exemplarIndex) {
	glm::mat4 mat = glm::translate(getModelMatrix(), granules[granuleIndex].vectors[exemplarIndex].position);
	if (granules[granuleIndex].vectors[exemplarIndex].rotation != glm::vec3(0.0f)) {
		mat = glm::rotate(mat, workingRotationAngleDegree, granules[granuleIndex].vectors[exemplarIndex].rotation);
	}
	return glm::scale(mat, granules[granuleIndex].vectors[exemplarIndex].scale * 2.0f);  // why here is 2?
}

void GranuleModel::generateVaos() {
	for (int i = 0; i < granules.size(); i++) {
		vector<glm::mat4> matricies;
		vector<int> trianlgeIndecies;
		for (int j = 0; j < granules[i].vectors.size(); j++) {
			matricies.push_back(createModelMatrix(i, j));
			//trianlgeIndecies.push_back(granules[i].vectors[j].triangleIndex);
			trianlgeIndecies.push_back(0);
		}
		GLBuffers buffers = granules[i].granule->createVao(&matricies, &trianlgeIndecies);
		granules[i].vao = buffers.vao;
		granules[i].vbo = buffers.vbo;
		granules[i].initSize = buffers.initSize;
	}
}


void GranuleModel::renderGranularGeometry() {
	for (int i = 0; i < granules.size(); i++) {
		glBindVertexArray(granules[i].vao);
		granules[i].granule->renderGeometry(granules[i].vectors.size());
	}
}

void GranuleModel::renderGranular(Matricies matricies, Camera* camera, Lights lights) {
	Texture* texture = getTrianglesTexture();
	for (int i = 0; i < granules.size(); i++) {
		granules[i].granule->renderAmount(matricies, camera, lights, granules[i].vao, granules[i].vectors.size(), texture);
	}
}

GranuleVectors GranuleModel::getVectors(int granuleIndex, int exemplarIndex) {
	return granules[granuleIndex].vectors[exemplarIndex];
}

void GranuleModel::setVectors(int granuleIndex, int exemplarIndex, GranuleVectors vectors) {
	granules[granuleIndex].vectors[exemplarIndex].position = vectors.position;
	granules[granuleIndex].vectors[exemplarIndex].rotation = vectors.rotation;
}

void GranuleModel::updateVbos() {
	for (int i = 0; i < granules.size(); i++) {
		for (int j = 0; j < granules[i].vectors.size(); j++) {
			granules[i].vbo->changeData(&createModelMatrix(i, j), sizeof(glm::mat4), (UINT)granules[i].initSize + j * sizeof(glm::mat4));
		}
	}
}

void GranuleModel::updateVbo(int granuleIndex, int exemplarIndex) {
	granules[granuleIndex].vbo->changeData(&createModelMatrix(granuleIndex, exemplarIndex), sizeof(glm::mat4), (UINT)granules[granuleIndex].initSize + exemplarIndex * sizeof(glm::mat4));
}

float GranuleModel::getScale(int granuleIndex) {
	return scaleIndex * granules[granuleIndex].granule->getScale().x;
}

float GranuleModel::getProbability(int granuleIndex) {
	float sum = 0.0f;
	for (int i = 0; i < granules.size(); i++) {
		sum += *granules[i].granule->getProbabilityRef();
	}
	return *granules[granuleIndex].granule->getProbabilityRef() / sum;
}

vector<GranuleSettings>* GranuleModel::getGranules() {
	return &granules;
}

map<int, vector<GranuleVectors*>*>* GranuleModel::getTriangelMap() {
	return &triangleMap;
}

ModelTriangle GranuleModel::getTriangle(int index) {
	return triangles[index];
}

float GranuleModel::getMinScale() {
	float minScale = Octahedron::radius;
	for (int i = 0; i < granules.size(); i++) {
		if (getScale(i) < minScale) minScale = getScale(i);
	}
	return minScale;
}

float GranuleModel::getMaxScale() {
	float maxScale = 0;
	for (int i = 0; i < granules.size(); i++) {
		if (getScale(i) > maxScale) maxScale = getScale(i);
	}
	return maxScale;
}

float GranuleModel::getAvgScale() {
	float averageRadius = 0.0f;
	for (int i = 0; i < granules.size(); i++) {
		averageRadius += getScale(i);
	}
	return averageRadius / (float)granules.size();
}

vector<float> GranuleModel::getScales() {
	vector<float> result;
	for (int i = 0; i < granules.size(); i++) {
		result.push_back(getScale(i));
	}
	return result;
}

ModelStatus GranuleModel::getStatus() {
	return status;
}

void GranuleModel::setStatus(ModelStatus status) {
	this->status = status;
}

int* GranuleModel::getMaxTrianglesRef() {
	return &maxTriangles;
}

float* GranuleModel::getVolumeLengthRef() {
	return &volumeLength;
}

float* GranuleModel::getMinLengthRef() {
	return &minLength;
}

float* GranuleModel::getMaxLengthRef() {
	return &maxLength;
}

float* GranuleModel::getScaleIndexRef() {
	return &scaleIndex;
}

void GranuleModel::setRenderingSettings(bool renderSettings) {
	this->renderSettings = renderSettings;
}

bool GranuleModel::isRenderingSettings() {
	return renderSettings;
}

int GranuleModel::getTriangleAmount() {
	return getModel()->getIndicies()->size() / 3;
}

Texture* GranuleModel::getTrianglesTexture() {
	/*vector<bool> data(getTriangleAmount());
	Texture* texture = &Granule::getTriangleTexture(data);*/
	return NULL;
}