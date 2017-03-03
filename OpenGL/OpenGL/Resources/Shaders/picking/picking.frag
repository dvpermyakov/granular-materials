#version 410

uniform float objectIndex;
uniform float meshIndex;

void main() {
	gl_FragColor = vec4(objectIndex, meshIndex, 0, 1);
}