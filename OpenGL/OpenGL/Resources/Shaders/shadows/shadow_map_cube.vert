#version 330 core

layout (location = 0) in vec3 inPosition;

uniform mat4 modelMatrix;

void main()
{
    gl_Position = modelMatrix * vec4(inPosition, 1.0);
}