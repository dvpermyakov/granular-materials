#version 330

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 theNormal[];

const float LINE_LENGTH = 0.2;

void generateLine(int index) {
    gl_Position = gl_in[index].gl_Position;
	EmitVertex();
	gl_Position = gl_in[index].gl_Position + vec4(theNormal[index], 0) * LINE_LENGTH;
	EmitVertex();
	EndPrimitive();
}

void main() {
    generateLine(0);
	generateLine(1);
	generateLine(2);
}