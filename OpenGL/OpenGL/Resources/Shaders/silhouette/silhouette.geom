#version 330

layout (triangles_adjacency) in;
layout (line_strip, max_vertices = 18) out;

in vec3 theWorldPosition[];
in vec3 theNormal[];

uniform vec3 lightPosition;

const float EPSILON = 0.0;

void generateLine(int index1, int index2) {
    gl_Position = gl_in[index1].gl_Position;
	EmitVertex();
	gl_Position = gl_in[index2].gl_Position;
	EmitVertex();
	EndPrimitive();
}

void main() {
    vec3 e1 = theWorldPosition[2] - theWorldPosition[0];
    vec3 e2 = theWorldPosition[4] - theWorldPosition[0];
    vec3 e3 = theWorldPosition[1] - theWorldPosition[0];
    vec3 e4 = theWorldPosition[3] - theWorldPosition[2];
    vec3 e5 = theWorldPosition[4] - theWorldPosition[2];
    vec3 e6 = theWorldPosition[5] - theWorldPosition[0];
 
    vec3 normal = cross(e1, e2);
    vec3 lightDirection = lightPosition - theWorldPosition[0];

	float sign = 1.0;
	if (dot(normal, lightDirection) < EPSILON) sign = -1.0;
 
    if (dot(theNormal[0], lightDirection) > EPSILON || dot(theNormal[2], lightDirection) > EPSILON || dot(theNormal[4], lightDirection) > EPSILON) {
		
		normal = cross(e3, e1);
		lightDirection = lightPosition - theWorldPosition[0];
        if (dot(normal, lightDirection) * sign <= EPSILON) {
            generateLine(0, 2);
        }
 
        normal = cross(e4, e5);
        lightDirection = lightPosition - theWorldPosition[2];
        if (dot(normal, lightDirection) * sign <= EPSILON) {
            generateLine(2, 4);
        }
 
        normal = cross(e2, e6);
        lightDirection = lightPosition - theWorldPosition[0];
        if (dot(normal, lightDirection) * sign <= EPSILON) {
            generateLine(4, 0);
        }
		
    }
}