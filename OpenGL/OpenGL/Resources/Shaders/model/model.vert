#version 330

const int MAX_BONES = 100;

uniform mat4 projectionMatrix;          // матрица проекции
uniform mat4 viewMatrix;                // матрица камеры
uniform mat4 modelMatrix;               // матрица объекта
uniform mat4 lightSpaceMatrix;          // матрица проекции * камеры источника света
uniform mat4 boneMatricies[MAX_BONES];  // матрицы всех костей
uniform bool withBones;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec4 inBoneIndecies;  // 4 indecies for 4 bones
layout (location = 4) in vec4 inBoneWeights;   // 4 weights  for 4 bones

out vec2 theTexCoord;
out vec3 theNormal;
out vec4 thePosition;
out vec4 theWorldPosition;
out vec4 theLightPosition;

void main()
{
	mat4 totalBoneMatrix; 
	if (withBones) {
		 totalBoneMatrix =  boneMatricies[int(inBoneIndecies[0])] * inBoneWeights[0];
	     totalBoneMatrix += boneMatricies[int(inBoneIndecies[1])] * inBoneWeights[1];
	     totalBoneMatrix += boneMatricies[int(inBoneIndecies[2])] * inBoneWeights[2];
	     //totalBoneMatrix += boneMatricies[int(inBoneIndecies[3])] * inBoneWeights[3];
	} 
	else {
		totalBoneMatrix = mat4(1.0);
	}
	theWorldPosition = modelMatrix * totalBoneMatrix * vec4(inPosition, 1.0);
	theLightPosition = lightSpaceMatrix * theWorldPosition;
	thePosition = projectionMatrix * viewMatrix * theWorldPosition;
	gl_Position = thePosition;
	theTexCoord = inTexCoord;
	theNormal = normalize(mat3(modelMatrix) * inNormal);
}