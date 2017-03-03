#version 330 core

uniform vec3 lightPosition;
uniform float projectionFarPlane;

in vec4 theWorldPosition;

void main()
{
    float distance = length(vec3(theWorldPosition) - lightPosition) / projectionFarPlane;
	gl_FragColor = vec4(vec3(distance), 1);
}  