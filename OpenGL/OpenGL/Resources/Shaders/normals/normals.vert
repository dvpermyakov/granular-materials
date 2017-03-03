#version 330

uniform mat4 projectionMatrix; // матрица проекции
uniform mat4 viewMatrix;       // матрица камеры
uniform mat4 modelMatrix;      // матрица объекта

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;

out vec2 theTexCoord;
out vec3 theNormal;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(inPosition, 1.0);
	theTexCoord = inTexCoord;
	mat3 normalMatrix = mat3(transpose(inverse(viewMatrix * modelMatrix)));
	theNormal = vec3(projectionMatrix * vec4(normalMatrix * inNormal, 1.0));
	theNormal = normalize(theNormal);
}