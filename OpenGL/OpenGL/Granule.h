#pragma once

#include "Buffer.h"
#include "ShaderProgram.h"
#include "Geometry.h"
#include "Light.h"
#include "Environment.h"
#include "Texture.h"
#include "Resources.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>

#include <vector>
#include <map>
#include <set>
#include <stdlib.h> 
#include <thread> 

using namespace std;

struct GLBuffers {
	GLuint vao;
	VertexBufferObject* vbo;
	int initSize;
};

struct GranuleGeometrySettings {
	int fragCount;
	int smoothCount;
	float sharpDegree;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & fragCount;
		ar & smoothCount;
		ar & sharpDegree;
	}
};

class GranuleGeometry {
public:
	GranuleGeometry();
	GranuleGeometry(GranuleGeometrySettings settings);
	void generateBuffers();
	void generate();
	GLBuffers createVao(vector<glm::mat4>* modelMatricies, vector<int>* triangleIndecies);
	void render(int amount);
	VertexBufferObject* getVBO();
	VertexBufferObject* getIndicies();
	GranuleGeometrySettings* getGranuleGeometrySettings();
	float getScale();
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version) { ar & scale; ar & settings; ar & indexCount; ar & vbo; ar & indicies; ar & pointCount; }
	float scale;
	GranuleGeometrySettings settings;
	int indexCount;
	int pointCount;
	VertexBufferObject vbo;
	VertexBufferObject indicies;
};

enum TextureType {
	CLAY = 0,
	MOSS,
	PEBBLE,
	ROCK,
	SAND,
};

class Granule : public AbstractModel {
public:
	static void loadShaders();
	Granule();
	Granule(string name, glm::vec3 color, glm::vec3 position, TextureType type, float shininess, float probability);
	GLBuffers createVao(vector<glm::mat4>* modelMatricies, vector<int>* triangleIndecies);
	VertexBufferObject* getVBO();
	VertexBufferObject* getIndicies();
	glm::mat4 getModelMatrix();
	glm::vec3 getScale();
	void doAction();
	void renderGeometry();
	void renderGeometry(int amount);
	void render(Matricies matricies, Camera* camera, Lights lights, bool onlyAmbient = false);
	void renderAmount(Matricies matricies, Camera* camera, Lights lights, GLuint vao, int amount, Texture* triangleTexture);
	bool isRenderingSettings();
	void setRenderingSettings(bool renderSettings);
	void setTexture();
	static Texture getTriangleTexture(vector<bool> data);
	float* getShininessRef();
	TextureType* getTextureTypeRef();
	glm::vec3* getColorRef();
	float* getProbabilityRef();
	GranuleGeometrySettings* getGranuleGeometrySettings();
	void generateGeometry(bool toGenerate = true);
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){ ar & boost::serialization::base_object<AbstractModel>(*this); ar & color; ar & probability;  ar & geometry; ar & type; ar & shininess; }
	static ShaderProgram program;
	TextureType type;
	float shininess;
	Texture colorTexture;
	Texture normalTexture;
	glm::vec3 color;
	float probability;
	GLuint vao;
	GranuleGeometry geometry;
	Texture triangleTexture;
	bool renderSettings;
};

struct GranuleVectors {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	int granuleIndex;
	int exemplarIndex;
	//int triangleIndex;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & position;
		ar & rotation;
		ar & scale;
		ar & granuleIndex;
		ar & exemplarIndex;
		//ar & triangleIndex;
	}
};

struct ModelTriangle {
	vector<glm::vec3> positions;
	glm::vec3 normal;
	float minLength;
	float maxLength;
};

struct GranuleSettings {
	vector<GranuleVectors> vectors;
	GLuint vao;
	VertexBufferObject* vbo;
	int initSize;
	Granule* granule;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & vectors;
		ar & vbo;
		ar & initSize;
		ar & granule;
	}
};

enum ModelStatus {
	EMPTY = 0,
	WITH_GRANULE_TYPES,
	WITH_EXEMPLARS,
	COMPRESSED,
};

class GranuleModel : public EnvironmentModel {
public:
	GranuleModel();
	void doAction();
	void loadGranules();
	void setGranules(vector<Granule*> granuleTypes);
	void setTriangles();
	void setGranulesExemplars();
	void renderGranularGeometry();
	void renderGranular(Matricies matricies, Camera* camera, Lights lights);
	GranuleVectors getVectors(int granuleIndex, int exemplarIndex);
	void setVectors(int granuleIndex, int exemplarIndex, GranuleVectors vectors);
	float getScale(int granuleIndex);
	float getProbability(int granuleIndex);
	void updateVbos();
	void updateVbo(int granuleIndex, int exemplarIndex);
	vector<GranuleSettings>* getGranules();
	map<int, vector<GranuleVectors*>*>* getTriangelMap();
	ModelTriangle getTriangle(int index);
	float getMinScale();
	float getMaxScale();
	float getAvgScale();
	vector<float> getScales();
	ModelStatus getStatus();
	void setStatus(ModelStatus status);
	int* getMaxTrianglesRef();
	float* getVolumeLengthRef();
	float* getMinLengthRef();
	float* getMaxLengthRef();
	float* getScaleIndexRef();
	void setRenderingSettings(bool renderSettings);
	bool isRenderingSettings();
	string getName();
	int getTriangleAmount();
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) { 
		ar & boost::serialization::base_object<EnvironmentModel>(*this);
		ar & status;
		ar & volumeLength;
		ar & minLength;
		ar & maxLength;
		ar & scaleIndex;
		ar & granules;
		ar & triangleMap;
	}
	void addGranulesInTriangle(int triangleIndex, TrianglePoints points, glm::vec3 normal, float minLength, float maxLength);
	void addGranule(glm::vec3 position, glm::vec3 scale, int granuleIndex, int triangleIndex);
	void generateVaos();
	glm::mat4 createModelMatrix(int granuleIndex, int exemplarIndex);
	Texture* getTrianglesTexture();
	int maxTriangles;
	ModelStatus status;
	float volumeLength;
	float minLength;
	float maxLength;
	float scaleIndex;
	vector<Granule*> granuleTypes;
	vector<ModelTriangle> triangles;
	vector<GranuleSettings> granules;
	map<int, vector<GranuleVectors*>*> triangleMap;
	bool renderSettings;
};