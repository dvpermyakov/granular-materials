#version 330

uniform mat4 projectionMatrix; // матрица проекции
uniform mat4 viewMatrix;       // матрица камеры

layout (location = 0) in vec3 inPosition;

out vec3 theTexCoord;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * vec4(inPosition, 1.0);
	theTexCoord = normalize(inPosition);
}