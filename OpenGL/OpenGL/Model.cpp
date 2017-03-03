#include "Model.h"

using namespace std;

// return directory path from file path
string getDirectory(string filePath) {
	for (int i = filePath.size() - 1; i >= 0; i--) {
		if (filePath[i] == '\\' || filePath[i] == '/') {
			return filePath.substr(0, i + 1);
		}
	}
	return "";
}

glm::mat4 toMat4(aiMatrix4x4 matrix) {
	return glm::transpose(glm::make_mat4(&matrix.a1));
}

glm::vec3 toVec3(aiVector3D vector) {
	return glm::vec3(vector.x, vector.y, vector.z);
}

Mesh::Mesh(int vertecies, int startingIndex, int startingVertex, int materialIndex) :
	vertecies(vertecies),
	startingIndex(startingIndex), 
	startingVertex(startingVertex),
	materialIndex(materialIndex) {}

int Mesh::getMaterialIndex() {
	return materialIndex;
}

int Mesh::getStartingIndex() {
	return startingIndex;
}

int Mesh::getStartingVertex() {
	return startingVertex;
}

int Mesh::getVertecies() {
	return vertecies;
}

void Mesh::render() {
	glDrawElementsBaseVertex(GL_TRIANGLES, vertecies, GL_UNSIGNED_INT, (void*)(sizeof(UINT) * startingIndex), startingVertex);
}

AdjacencyMesh::AdjacencyMesh(int vertecies, int startingIndex, int startingVertex, int materialIndex) : Mesh(vertecies, startingIndex, startingVertex, materialIndex) {}

void AdjacencyMesh::render() {
	//glDrawElementsBaseVertex(GL_TRIANGLES_ADJACENCY, vertecies, GL_UNSIGNED_INT, (void*)(sizeof(UINT) * startingIndex), startingVertex);
	glDrawElementsBaseVertex(GL_TRIANGLES_ADJACENCY, vertecies, GL_UNSIGNED_INT, (void*)(sizeof(UINT) * startingIndex), startingVertex);
}

Bone::Bone(string name, vector<BoneVertex*> vertecies, aiMatrix4x4 matrix) : name(name), vertecies(vertecies), matrix(matrix), transformation(glm::mat4(0)) {}

glm::mat4 Bone::getMatrix() {
	return toMat4(matrix);
}

void Bone::saveTransformation(glm::mat4 matrix) {
	transformation = matrix;
}

glm::mat4 Bone::getTransformation() {
	return transformation;
}


vector<BoneVertex*> Bone::getVertecies() {
	return vertecies;
}

Material::Material() : shininess(256.0f) {}

void Material::saveAmbientTexture(string path) {
	ambient.loadFromPath(path, true);
	ambient.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ambient.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
}

void Material::saveDiffuseTexture(string path) {
	diffuse.loadFromPath(path, true);
	diffuse.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	diffuse.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
}

void Material::saveSpecularTexture(string path) {
	specular.loadFromPath(path, true);
	specular.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	specular.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
}

Texture Material::getAmbientTexture() {
	return ambient;
}

Texture Material::getDiffuseTexture() {
	return diffuse;
}

Texture Material::getSpecularTexture() {
	return specular;
}

float Material::getShininess() {
	return shininess;
}

void Material::release() {
	if (ambient.wasCreated()) {
		ambient.unbindTexture();
	}
	if (diffuse.wasCreated()) {
		diffuse.unbindTexture();
	}
	if (specular.wasCreated()) {
		specular.unbindTexture();
	}
}

Model::Model() : vao(0), adjacencyVao(0), totalIndicies(0), totalVertecies(0) {}

const aiScene* Model::getScene(string filePath) {
	const aiScene* scene =  importer.ReadFile(filePath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);
	return scene;
}

bool Model::loadModelFromFile(string filePath, bool generateAdjacency) {
	const aiScene* scene = getScene(filePath);
	if (!scene) {
		MessageBox(appMain.hWnd, "Couldn't load model", "Error Importing Asset", MB_ICONERROR);
		return false;
	}
	this->filePath = filePath;
	vbo.generate();
	indicies.generate();
	adjacencyIndicies.generate();
	int currentIndex = 0;
	int currentVertex = 0;
	vector<int> currentAdjacencyIndecies;
	map<pair<int, int>, set<int>> adjacencies;
	for (int i = 0; i < scene->mNumMeshes; i++) {  // iterate meshes
		aiMesh* mesh = scene->mMeshes[i];
		for (int j = 0; j < mesh->mNumVertices; j++) {
			aiVector3D pos = mesh->mVertices[j];
			aiVector3D uv = mesh->mTextureCoords[0][j];
			aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[j] : aiVector3D(1.0f, 1.0f, 1.0f);
			vbo.addData(&pos, sizeof(aiVector3D));
			vbo.addData(&uv, sizeof(aiVector2D));
			vbo.addData(&normal, sizeof(aiVector3D));
			vertecies.push_back(toVec3(pos));
			normals.push_back(toVec3(normal));
		}
		for (int j = 0; j < mesh->mNumFaces; j++) {
			const aiFace& face = mesh->mFaces[j];
			indicies.addData(&face.mIndices[0], sizeof(UINT)); indiciesArray.push_back(face.mIndices[0]);
			indicies.addData(&face.mIndices[1], sizeof(UINT)); indiciesArray.push_back(face.mIndices[1]);
			indicies.addData(&face.mIndices[2], sizeof(UINT)); indiciesArray.push_back(face.mIndices[2]);

			if (generateAdjacency) {
				int ai1 = face.mIndices[0] + currentVertex, ai2 = face.mIndices[1] + currentVertex, ai3 = face.mIndices[2] + currentVertex;
				for (int k = currentVertex; k < vertecies.size(); k++) {
					if (k < ai1 && vertecies[k] == vertecies[ai1]) ai1 = k;
					if (k < ai2 && vertecies[k] == vertecies[ai2]) ai2 = k;
					if (k < ai3 && vertecies[k] == vertecies[ai3]) ai3 = k;
					if (k > ai1 && k > ai2 && k > ai3) break;
				}
				ai1 -= currentVertex;
				ai2 -= currentVertex;
				ai3 -= currentVertex;
				if (ai1 > ai2) swap(ai1, ai2);
				if (ai2 > ai3) swap(ai2, ai3);
				if (ai1 > ai2) swap(ai1, ai2);
				currentAdjacencyIndecies.push_back(ai1);
				currentAdjacencyIndecies.push_back(ai2);
				currentAdjacencyIndecies.push_back(ai3);
				// 1
				if (adjacencies.find(pair<int, int>(ai1, ai2)) == adjacencies.end()) {
					adjacencies[pair<int, int>(ai1, ai2)] = set<int>();
				}
				adjacencies[pair<int, int>(ai1, ai2)].insert(ai3);
				// 2
				if (adjacencies.find(pair<int, int>(ai2, ai3)) == adjacencies.end()) {
					adjacencies[pair<int, int>(ai2, ai3)] = set<int>();
				}
				adjacencies[pair<int, int>(ai2, ai3)].insert(ai1);
				// 3
				if (adjacencies.find(pair<int, int>(ai1, ai3)) == adjacencies.end()) {
					adjacencies[pair<int, int>(ai1, ai3)] = set<int>();
				}
				adjacencies[pair<int, int>(ai1, ai3)].insert(ai2);
			}
		}
		map<string, Bone*> meshBones;
		for (int j = 0; j < mesh->mNumBones; j++) {
			aiBone* bone = mesh->mBones[j];
			vector<BoneVertex*> boneVertexes;
			for (int u = 0; u < bone->mNumWeights; u++) {
				BoneVertex* boneVertex = new BoneVertex();
				boneVertex->vertexId = bone->mWeights[u].mVertexId + currentVertex;
				boneVertex->weight = bone->mWeights[u].mWeight;
				boneVertexes.push_back(boneVertex);
			}
			string boneName = bone->mName.data;
			meshBones[boneName] = new Bone(boneName, boneVertexes, bone->mOffsetMatrix);
		}
		Mesh meshObj(mesh->mNumFaces * 3, currentIndex, currentVertex, mesh->mMaterialIndex);
		bones.insert(meshBones.begin(), meshBones.end());
		currentIndex += mesh->mNumFaces * 3;
		currentVertex += mesh->mNumVertices;
		meshes.push_back(meshObj);  // save in list of meshes
		AdjacencyMesh adMesh(meshObj.getVertecies() * 2, meshObj.getStartingIndex() * 2, meshObj.getStartingVertex(), meshObj.getMaterialIndex());
		adjacencyMeshes.push_back(adMesh);
	}

	if (generateAdjacency) {
		for (int j = 0; j < currentAdjacencyIndecies.size(); j += 3) {
			int ai1 = currentAdjacencyIndecies[j], ai2 = currentAdjacencyIndecies[j + 1], ai3 = currentAdjacencyIndecies[j + 2];
			UINT i1 = *adjacencies[pair<int, int>(ai1, ai2)].begin() != ai3 ? *adjacencies[pair<int, int>(ai1, ai2)].begin() : *adjacencies[pair<int, int>(ai1, ai2)].rbegin(),
				i2 = *adjacencies[pair<int, int>(ai2, ai3)].begin() != ai1 ? *adjacencies[pair<int, int>(ai2, ai3)].begin() : *adjacencies[pair<int, int>(ai2, ai3)].rbegin(),
				i3 = *adjacencies[pair<int, int>(ai1, ai3)].begin() != ai2 ? *adjacencies[pair<int, int>(ai1, ai3)].begin() : *adjacencies[pair<int, int>(ai1, ai3)].rbegin();
			adjacencyIndicies.addData(&ai1, sizeof(UINT));
			adjacencyIndicies.addData(&i1, sizeof(UINT));
			adjacencyIndicies.addData(&ai2, sizeof(UINT));
			adjacencyIndicies.addData(&i2, sizeof(UINT));
			adjacencyIndicies.addData(&ai3, sizeof(UINT));
			adjacencyIndicies.addData(&i3, sizeof(UINT));
		}
	}

 	totalIndicies = currentIndex;
	totalVertecies = currentVertex;
	map<int, vector<ReverseBoneVertex>> vertexBones;
	int index = 0;
	for (map<string, Bone*>::iterator iterator = bones.begin(); iterator != bones.end(); iterator++, index++) {
		vector<BoneVertex*> vertecies = iterator->second->getVertecies();
		for (int i = 0; i < vertecies.size(); i++) {
			ReverseBoneVertex reverseVertex;
			reverseVertex.boneId = index;
			reverseVertex.weight = vertecies[i]->weight;
			vertexBones[vertecies[i]->vertexId].push_back(reverseVertex);
		}
	}
	for (int i = 0; i < totalVertecies; i++) {
		ReverseBoneVertex init;
		init.boneId = 0;
		init.weight = 0.0f;
		while (vertexBones[i].size() < 4) {
			vertexBones[i].push_back(init);
		}
	}
	for (int i = 0; i < totalVertecies; i++) {
		vector<ReverseBoneVertex> reverseBonesVerteceis = vertexBones[i];
		int boneIds[4] = {
			reverseBonesVerteceis[0].boneId,
			reverseBonesVerteceis[1].boneId,
			reverseBonesVerteceis[2].boneId,
			reverseBonesVerteceis[3].boneId
		};
		vbo.addData(&boneIds, sizeof(boneIds));
		float weights[4] = {
			reverseBonesVerteceis[0].weight,
			reverseBonesVerteceis[1].weight,
			reverseBonesVerteceis[2].weight,
			reverseBonesVerteceis[3].weight
		};
		vbo.addData(&weights, sizeof(weights));
	}

	for (int i = 0; i < scene->mNumMaterials; i++) {
		const aiMaterial* material = scene->mMaterials[i];
		aiString path;  // retrieved from GetTexture method
		Material materialObj;
		if (material->GetTexture(aiTextureType_AMBIENT, 0, &path) == AI_SUCCESS) {  // ambient texture is found
			string fullPath = getDirectory(filePath) + path.data;
			materialObj.saveAmbientTexture(fullPath);
		}
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {  // difuse texture is found
			string fullPath = getDirectory(filePath) + path.data;
			materialObj.saveDiffuseTexture(fullPath);
		}
		if (material->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS) {  // specular texture is found
			string fullPath = getDirectory(filePath) + path.data;
			materialObj.saveSpecularTexture(fullPath);
		}
		materials.push_back(materialObj);
	}
	return true;
}

void Model::generateVao() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	vbo.bind();
	vbo.upload(GL_STATIC_DRAW, false);
	indicies.bind(GL_ELEMENT_ARRAY_BUFFER);
	indicies.upload(GL_STATIC_DRAW, false);
	float stride = 2 * sizeof(aiVector3D) + sizeof(aiVector2D);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(aiVector3D));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(aiVector3D) + sizeof(aiVector2D)));
	float offset = stride * totalVertecies;
	stride = sizeof(int[4]) + sizeof(float[4]);
	// Bone ids
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, stride, (void*)sizeof(offset));
	// Bone weight
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(offset) + sizeof(int[4])));
}

void Model::generateAdjacencyVao() {
	glGenVertexArrays(1, &adjacencyVao);
	glBindVertexArray(adjacencyVao);
	vbo.bind();
	vbo.upload(GL_STATIC_DRAW, false);
	adjacencyIndicies.bind(GL_ELEMENT_ARRAY_BUFFER);
	adjacencyIndicies.upload(GL_STATIC_DRAW, false);
	float stride = 2 * sizeof(aiVector3D) + sizeof(aiVector2D);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(aiVector3D));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(aiVector3D) + sizeof(aiVector2D)));
	float offset = stride * totalVertecies;
	stride = sizeof(int[4]) + sizeof(float[4]);
	// Bone ids
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, stride, (void*)sizeof(offset));
	// Bone weight
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(offset) + sizeof(int[4])));
}

void Model::bindVao() {
	glBindVertexArray(vao);
}

void Model::bindAdjacencyVao() {
	glBindVertexArray(adjacencyVao);
}

void Model::renderGeometry() {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].render();
	}
}

void Model::renderAdjacencyGeometry() {
	for (int i = 0; i < adjacencyMeshes.size(); i++) {
		adjacencyMeshes[i].render();
	}
}

void Model::renderMesh(int index) {
	meshes[index].render();
}

int Model::getMeshesSize() {
	return meshes.size();
}

int Model::getMesh(int index) {
	for (int i = 1; i < meshes.size(); i++) {
		if (meshes[i].getStartingIndex() > index) return i - 1;
	}
	return meshes.size() - 1;
}

Material* Model::getMaterial(int meshIndex) {
	return &materials[meshes[meshIndex].getMaterialIndex()];
}

map<string, Bone*>* Model::getBones() {
	return &bones;
}

const aiNodeAnim* Model::getNodeAnimation(const aiAnimation* animation, const string nodeName) {
	for (int i = 0; i < animation->mNumChannels; i++) {
		const aiNodeAnim* nodeAnimation = animation->mChannels[i];
		if (string(nodeAnimation->mNodeName.data) == nodeName) {
			return nodeAnimation;
		}
	}
	return NULL;
}

int Model::getPositionKeyIndex(float time, const aiNodeAnim* nodeAnimation) {
	for (int i = 1; i < nodeAnimation->mNumPositionKeys; i++) {
		if (time < (float)nodeAnimation->mPositionKeys[i].mTime) {
			return i - 1;
		}
	}
	return -1;
}

int Model::getRotationKeyIndex(float time, const aiNodeAnim* nodeAnimation) {
	for (int i = 1; i < nodeAnimation->mNumRotationKeys; i++) { 
		if (time < (float)nodeAnimation->mRotationKeys[i].mTime) {
			return i - 1;
		}
	}
	return -1;
}

int Model::getScalingKeyIndex(float time, const aiNodeAnim* nodeAnimation) {
	for (int i = 1; i < nodeAnimation->mNumScalingKeys; i++) {
		if (time < (float)nodeAnimation->mScalingKeys[i].mTime) {
			return i - 1;
		}
	}
	return -1;
}

aiVector3D* Model::getPosition(float time, const aiNodeAnim* nodeAnimation) {
	if (nodeAnimation->mNumPositionKeys == 1) {
		return &nodeAnimation->mPositionKeys[0].mValue;
	}

	int positionIndex = getPositionKeyIndex(time, nodeAnimation);
	int nextPositionIndex = (positionIndex + 1);
	float deltaTime = (float)(nodeAnimation->mPositionKeys[nextPositionIndex].mTime - nodeAnimation->mPositionKeys[positionIndex].mTime);
	float previousTime = (float)nodeAnimation->mPositionKeys[positionIndex].mTime;
	float factor = (time - previousTime) / deltaTime;
	if (factor < 0 || factor > 1.0) {
		return NULL;
	}
	const aiVector3D& Start = nodeAnimation->mPositionKeys[positionIndex].mValue;
	const aiVector3D& End = nodeAnimation->mPositionKeys[nextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	return &(Start + factor * Delta);
}

aiQuaternion* Model::getRotation(float time, const aiNodeAnim* nodeAnimation) {
	if (nodeAnimation->mNumRotationKeys == 1) {
		return &nodeAnimation->mRotationKeys[0].mValue;
	}

	aiQuaternion* vector = new aiQuaternion();
	int rotationIndex = getRotationKeyIndex(time, nodeAnimation);
	int nextRotationIndex = (rotationIndex + 1);
	float deltaTime = nodeAnimation->mRotationKeys[nextRotationIndex].mTime - nodeAnimation->mRotationKeys[rotationIndex].mTime;

	float factor = (time - (float)nodeAnimation->mRotationKeys[rotationIndex].mTime) / deltaTime;
	if (factor < 0 || factor > 1.0) {
		return NULL;
	}
	const aiQuaternion& StartRotationQ = nodeAnimation->mRotationKeys[rotationIndex].mValue;
	const aiQuaternion& EndRotationQ = nodeAnimation->mRotationKeys[nextRotationIndex].mValue;
	aiQuaternion::Interpolate(*vector, StartRotationQ, EndRotationQ, factor);
	return &vector->Normalize();
}

aiVector3D* Model::getScaling(float time, const aiNodeAnim* nodeAnimation) {
	if (nodeAnimation->mNumScalingKeys == 1) {
		return &nodeAnimation->mScalingKeys[0].mValue;
	}

	int scalingIndex = getScalingKeyIndex(time, nodeAnimation);
	int nextScalingIndex = (scalingIndex + 1);
	float deltaTime = (float)(nodeAnimation->mScalingKeys[nextScalingIndex].mTime - nodeAnimation->mScalingKeys[scalingIndex].mTime);
	float factor = (time - (float)nodeAnimation->mScalingKeys[scalingIndex].mTime) / deltaTime;
	if (factor < 0 || factor > 1.0) {
		return NULL;
	}
	const aiVector3D& Start = nodeAnimation->mScalingKeys[scalingIndex].mValue;
	const aiVector3D& End = nodeAnimation->mScalingKeys[nextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	return &(Start + factor * Delta);
}

void Model::setNodeTransformation(float time, const aiAnimation* animation, const aiNode* node, glm::mat4 parentMatrix) {  // node name should be equal to bone name
	glm::mat4 nodeMatrix = toMat4(node->mTransformation);
	if (bones.find(node->mName.data) != bones.end()) {  // if we stores bone which relates to this node
		const aiNodeAnim* nodeAnimation = getNodeAnimation(animation, node->mName.data);  // find nodeAnimation for this animation
		if (nodeAnimation) {  // if node participate in this animation
			aiVector3D* scalingVector = &nodeAnimation->mScalingKeys[0].mValue;
			//aiVector3D* scalingVector = getScaling(time, nodeAnimation);
			glm::mat4 scalingMatrix = glm::scale(toVec3(*scalingVector));

			aiMatrix3x3 rotationAiMatrix = nodeAnimation->mRotationKeys[0].mValue.GetMatrix();
			//aiMatrix3x3 rotationAiMatrix = getRotation(time, nodeAnimation)->GetMatrix();
			glm::mat4 rotationMatrix = toMat4(aiMatrix4x4(rotationAiMatrix));

			aiVector3D* positionVector = &nodeAnimation->mPositionKeys[0].mValue;
			//aiVector3D* positionVector = getPosition(time, nodeAnimation);
			glm::mat4 positoinMatrix = glm::translate(toVec3(*positionVector));

			nodeMatrix = positoinMatrix * rotationMatrix * scalingMatrix;
		}
		Bone* bone = bones[node->mName.data];
		glm::mat4 matrix = parentMatrix * nodeMatrix * bone->getMatrix();
		bone->saveTransformation(matrix);
	}
	for (int i = 0; i < node->mNumChildren; i++) {
		setNodeTransformation(time, animation, node->mChildren[i], parentMatrix * nodeMatrix);
	}
}

void Model::setAnimation(float time, int animationNumber) {
	const aiScene* scene = getScene(filePath);
	if (animationNumber >= 0 && scene->mNumAnimations > animationNumber) {
		const aiAnimation* animation = scene->mAnimations[animationNumber];
		time = fmod(time * (animation->mTicksPerSecond / 2) , animation->mDuration);  // float mod
		setNodeTransformation(time, animation, scene->mRootNode, glm::mat4(1));
	}
}

string Model::getPath() {
	return filePath;
}

vector<glm::vec3>* Model::getVertecies() {
	return &vertecies;
}

vector<glm::vec3>* Model::getNormals() {
	return &normals;
}

vector<UINT>* Model::getIndicies() {
	return &indiciesArray;
}

bool Model::isLoaded() {
	return vao != 0;
}

void Model::release() {
	for (int i = 0; i < materials.size(); i++) {
		materials[i].release();
	}
	materials.clear();
	meshes.clear();
	vao = 0;
	totalIndicies = 0;
	totalVertecies = 0;
}