#version 330

uniform samplerCube cubeTexture;
uniform float intensity;

in vec3 theTexCoord;

void main() {
	gl_FragColor = (0.1 + intensity) * texture(cubeTexture, theTexCoord);
}