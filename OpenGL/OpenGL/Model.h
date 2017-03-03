#pragma once

#include "WindowApp.h"
#include "Buffer.h"
#include "Light.h"
#include "Camera.h"
#include "Texture.h"
#include "ShaderProgram.h"

#include <Windows.h>

#include <GL/glew.h>
#include <gl/wglew.h>

#include <assimp/Importer.hpp>      
#include <assimp/scene.h>           
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>

using namespace std;

struct BoneVertex {
	int vertexId;  // it relates to global vertex index in model
	float weight;
};

struct ReverseBoneVertex {
	int boneId;
	float weight;
};

class Bone {
public:
	Bone(string name, vector<BoneVertex*> vertecies, aiMatrix4x4 matrix);
	glm::mat4 getMatrix();
	void saveTransformation(glm::mat4 matrix);
	glm::mat4 getTransformation();
	vector<BoneVertex*> getVertecies();
private:
	string name;                     // it is identified by name
	vector<BoneVertex*> vertecies;   // it stores weights for vertecies
	aiMatrix4x4 matrix;              // it let us to move from local coordinates to bone coordinates
	glm::mat4 transformation;        // it stores current transformation matrix
};

class Mesh {
public:
	Mesh(int vertecies, int startingIndex, int startingVertex, int materialIndex);
	int getMaterialIndex();
	int getStartingIndex();
	int getStartingVertex();
	int getVertecies();
	void render();
protected:
	int vertecies;
	int startingIndex;
	int startingVertex;
	int materialIndex;
};

class AdjacencyMesh : public Mesh {
public:
	AdjacencyMesh(int vertecies, int startingIndex, int startingVertex, int materialIndex);
	void render();
};

class Material {
public:
	Material();
	void saveAmbientTexture(string path);
	void saveDiffuseTexture(string path);
	void saveSpecularTexture(string path);
	Texture getAmbientTexture();
	Texture getDiffuseTexture();
	Texture getSpecularTexture();
	float getShininess();
	void release();
private:
	Texture ambient;
	Texture diffuse;
	Texture specular;
	float shininess;
};

class Model {
public:
	Model();
	bool loadModelFromFile(string filePath, bool generateAdjacency = false);
	void generateVao();
	void generateAdjacencyVao();
	void bindVao();
	void bindAdjacencyVao();
	void renderGeometry();
	void renderAdjacencyGeometry();
	void renderMesh(int index);
	int getMeshesSize();
	int getMesh(int index);
	Material* getMaterial(int meshIndex);
	map<string, Bone*>* getBones();
	void setAnimation(float time, int animationNumber);
	string getPath();
	vector<glm::vec3>* getVertecies();
	vector<glm::vec3>* getNormals();
	vector<UINT>* getIndicies();
	bool isLoaded();
	void release();
private:
	static int getPositionKeyIndex(float time, const aiNodeAnim* nodeAnimation);
	static int getRotationKeyIndex(float time, const aiNodeAnim* nodeAnimation);
	static int getScalingKeyIndex(float time, const aiNodeAnim* nodeAnimation);
	static aiVector3D* getPosition(float time, const aiNodeAnim* nodeAnimation);
	static aiQuaternion* getRotation(float time, const aiNodeAnim* nodeAnimation);
	static aiVector3D* getScaling(float time, const aiNodeAnim* nodeAnimation);
	static const aiNodeAnim* getNodeAnimation(const aiAnimation* animation, const string nodeName);
	void setNodeTransformation(float time, const aiAnimation* animation, const aiNode* node, glm::mat4 parentMatrix);

	const aiScene* getScene(string filePath);
	Assimp::Importer importer;
	string filePath;
	GLuint vao;
	GLuint adjacencyVao;
	VertexBufferObject vbo;
	VertexBufferObject indicies;
	VertexBufferObject adjacencyIndicies;
	int totalIndicies;
	int totalVertecies;
	vector<glm::vec3> vertecies;
	vector<glm::vec3> normals;
	vector<UINT> indiciesArray;
	vector<Mesh> meshes;
	vector<AdjacencyMesh> adjacencyMeshes;
	vector<Material> materials;
	map<string, Bone*> bones;
};