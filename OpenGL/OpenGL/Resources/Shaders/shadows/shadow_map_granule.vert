#version 330

uniform mat4 projectionMatrix; // ������� ��������
uniform mat4 viewMatrix;       // ������� ������
uniform mat4 modelMatrix;      // ������� �������

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 tangent;
layout (location = 2) in int triangleIndex;
layout (location = 3) in mat4 instanceMatrix;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * instanceMatrix * vec4(inPosition, 1.0);
}