#pragma once

#include "Light.h"
#include "Model.h"
#include "Shadow.h"
#include "Resources.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>

#include <string>
#include <vector>

using namespace std;

const float workingRotationAngleDegree = 180.0f;
const float workingRotationAngleRadians = 3.14159f;

namespace boost {
	namespace serialization {
		template<class Archive>
		void serialize(Archive & ar, glm::vec3& vec, const unsigned int version) { ar & vec.x; ar & vec.y; ar & vec.z; }
	}
}

struct Matricies {
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 modelMatrix;
};

struct Lights {
	DirectLight* directLight;
	vector<PointLight*>* pointLights;
	Texture* shadowMap;
	vector<Texture*>* cubeShadowMaps;
};

class AbstractModel {
public:
	static void loadShaders();
	AbstractModel();
	virtual glm::mat4 getModelMatrix();
	virtual glm::vec3 getPosition();
	virtual glm::vec3 getRotation();
	virtual glm::vec3 getScale();
	virtual glm::vec3 getVelocity();
	virtual void setModelMatrix(glm::vec3* position, glm::vec3* rotation = NULL, glm::vec3* scale = NULL);
	virtual string getName();
	virtual void doAction();
	virtual void contactAction();
	virtual void release();
	virtual void renderNormals(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, glm::mat4 modelMatrix);

	virtual void renderGeometry() = 0;
	virtual void render(Matricies matricies, Camera* camera, Lights lights, bool additionalBool = false) = 0;
protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) { ar & name; ar & scale; ar & position; ar & rotation; }
	static ShaderProgram program;
	static ShaderProgram normalProgram;
	static ShaderProgram silhouetteProgram;
	virtual ShaderProgram* getProgram();
	void setShaderValues(Matricies matricies, Camera* camera, Lights lights);
	string name;
	glm::vec3 scale;
	glm::vec3 position;
	glm::vec3 rotation;
};

class EnvironmentModel : public AbstractModel {
public:
	EnvironmentModel();
	int getMeshesSize();
	string getPath();
	void renderMesh(int index);
	void renderGeometry();
	void renderSilhouetteGeometry();
	void renderSilhouette(Matricies matricies, Lights lights);
	virtual void render(Matricies matricies, Camera* camera, Lights lights, bool onlyAmbient = false);
	virtual void release();
	void load(string path, bool generateAdjacencies = false);
protected:
	virtual Model* getModel();
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) { ar & boost::serialization::base_object<AbstractModel>(*this); ar & path; }
	Model* model;
	string path;
};

class Lamp : public EnvironmentModel {
public:
	static void loadModel();
	Lamp(PointLight light);
	PointLight* getLight();
	glm::mat4 getModelMatrix();
	ShadowMap* getShadowMap();
	void doAction(); 
private:
	static Model lampModel;
	Model* getModel();
	PointLight light;
	ShadowMap shadowMap;
	int shadowLength;
};