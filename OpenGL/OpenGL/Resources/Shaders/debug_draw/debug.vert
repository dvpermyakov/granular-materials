#version 330

uniform mat4 projectionMatrix; // матрица проекции
uniform mat4 viewMatrix;       // матрица камеры

layout (location = 0) in vec3 inPosition;

void main() {
	gl_Position = projectionMatrix * viewMatrix * vec4(inPosition, 1.0);
}