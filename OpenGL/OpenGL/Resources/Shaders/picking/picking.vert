#version 410

uniform mat4 projectionMatrix; // матрица проекции
uniform mat4 viewMatrix;       // матрица камеры
uniform mat4 modelMatrix;      // матрица объекта

layout (location = 0) in vec3 inPosition;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(inPosition, 1.0);
}