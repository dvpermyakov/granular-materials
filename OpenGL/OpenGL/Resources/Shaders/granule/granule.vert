#version 330

uniform mat4 projectionMatrix; // матрица проекции
uniform mat4 viewMatrix;       // матрица камеры
uniform mat4 modelMatrix;      // матрица модели
uniform mat4 lightSpaceMatrix; // матрица проекции * камеры источника света

uniform bool useTriangleEnable;
uniform sampler1D triangleEnable;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 tangent;
layout (location = 2) in int triangleIndex;
layout (location = 3) in mat4 instanceMatrix;

out vec3 theNormal;
out vec4 theWorldPosition;
out vec4 theLightPosition;
out mat3 TBN;

void main() {
	if (!useTriangleEnable || abs(texture(triangleEnable, float(triangleIndex)).r - 1.0) < 0.0001) {
		theNormal = normalize(mat3(instanceMatrix) * inPosition);
		theWorldPosition = modelMatrix * instanceMatrix * vec4(inPosition, 1.0);
		theLightPosition = lightSpaceMatrix * theWorldPosition;
		gl_Position = projectionMatrix * viewMatrix * theWorldPosition;

		vec3 T = normalize(mat3(instanceMatrix) * tangent);
		vec3 N = normalize(mat3(instanceMatrix) * normalize(inPosition));
		vec3 B = normalize(mat3(instanceMatrix) * cross(T, N));
		mat3 TBN = mat3(T, B, N);
	} 
	else {
		gl_Position = vec4(2.0);
	}
}