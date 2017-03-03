#version 330

uniform sampler2D gSampler;

smooth in vec2 texCoord;
flat in vec4 colorPart;

void main() {
	vec4 texColor = texture2D(gSampler, texCoord);
	gl_FragColor = vec4(texColor.xyz, 1.0) * colorPart;
}