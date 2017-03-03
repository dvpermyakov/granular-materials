#version 410

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;     
uniform mat4 modelMatrix;     
uniform mat4 lightSpaceMatrix;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord; 
layout (location = 2) in vec3 inNormal;

out vec3 theLocalPosition;
out vec2 theTexCoord;
out vec3 theNormal;
out vec4 thePosition;
out vec4 theWorldPosition;
out vec4 theLightPosition;

void main()
{
	theLocalPosition = inPosition;
	theWorldPosition = modelMatrix * vec4(inPosition, 1.0);
	theLightPosition = lightSpaceMatrix * theWorldPosition;
	thePosition = projectionMatrix * viewMatrix * theWorldPosition;
	gl_Position = thePosition;
	theTexCoord = inTexCoord;
	//theNormal = normalize(mat3(modelMatrix) * inNormal);
	theNormal = inNormal;
}