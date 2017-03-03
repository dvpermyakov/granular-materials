#version 330

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 projectionMatrix; // матрица проекции
uniform mat4 viewMatrix;       // матрица камеры
uniform vec3 lightPosition;

in vec3 theWorldPosition[];
in vec3 theNormal[];

float BIAS = 0.01;
float EPSILON = 0.0;

void generateQuad(vec3 startVertex, vec3 endVertex) {
	mat4 gVP = projectionMatrix * viewMatrix;

    vec3 lightDirection = normalize(startVertex - lightPosition);
    vec3 eps = lightDirection * BIAS;
    gl_Position = gVP * vec4((startVertex + eps), 1.0);
    EmitVertex();
 
    gl_Position = gVP * vec4(lightDirection, 0.0);
    EmitVertex();
 
    lightDirection = normalize(endVertex - lightPosition);
    eps = lightDirection * BIAS;
    gl_Position = gVP * vec4((endVertex + eps), 1.0);
    EmitVertex();
 
    gl_Position = gVP * vec4(lightDirection, 0.0);
    EmitVertex();
 
    EndPrimitive();
}

void main() {
	mat4 gVP = projectionMatrix * viewMatrix;

    vec3 e1 = theWorldPosition[2] - theWorldPosition[0];
    vec3 e2 = theWorldPosition[4] - theWorldPosition[0];
    vec3 e3 = theWorldPosition[1] - theWorldPosition[0];
    vec3 e4 = theWorldPosition[3] - theWorldPosition[2];
    vec3 e5 = theWorldPosition[4] - theWorldPosition[2];
    vec3 e6 = theWorldPosition[5] - theWorldPosition[0];
 
    vec3 normal = cross(e1, e2);
    vec3 lightDirection = lightPosition- theWorldPosition[0];
	
	float sign = 1.0;
	if (dot(normal, lightDirection) < EPSILON) sign = -1.0;

    if (dot(theNormal[0], lightDirection) > EPSILON || dot(theNormal[2], lightDirection) > EPSILON || dot(theNormal[4], lightDirection) > EPSILON) {
 
        normal = cross(e3, e1);
        lightDirection = lightPosition - theWorldPosition[0];
        if (dot(normal, lightDirection) * sign <= EPSILON) {
            generateQuad(theWorldPosition[0], theWorldPosition[2]);
        }
 
        normal = cross(e4, e5);
        lightDirection = lightPosition - theWorldPosition[2];
        if (dot(normal, lightDirection) * sign <= EPSILON) {
            generateQuad(theWorldPosition[2], theWorldPosition[4]);
        }
 
        normal = cross(e2, e6);
        lightDirection = lightPosition - theWorldPosition[0];
        if (dot(normal, lightDirection) * sign <= EPSILON) {
            generateQuad(theWorldPosition[4], theWorldPosition[0]);
        }

		// верхнее основание
        lightDirection = (normalize(theWorldPosition[0] - lightPosition)) * BIAS;
        gl_Position = gVP * vec4((theWorldPosition[0] + lightDirection), 1.0);
        EmitVertex();
 
        lightDirection = (normalize(theWorldPosition[2] - lightPosition)) * BIAS;
        gl_Position = gVP * vec4((theWorldPosition[2] + lightDirection), 1.0);
        EmitVertex();
 
        lightDirection = (normalize(theWorldPosition[4] - lightPosition)) * BIAS;
        gl_Position = gVP * vec4((theWorldPosition[4] + lightDirection), 1.0);
        EmitVertex();
        EndPrimitive();
 
        // нижнее основание
        lightDirection = theWorldPosition[2] - lightPosition;
        gl_Position = gVP * vec4(lightDirection, 0.0);
        EmitVertex();
 
        lightDirection = theWorldPosition[0] - lightPosition;
        gl_Position = gVP * vec4(lightDirection, 0.0);
        EmitVertex();
 
        lightDirection = theWorldPosition[4] - lightPosition;
        gl_Position = gVP * vec4(lightDirection, 0.0);
        EmitVertex();
 
        EndPrimitive();
    }
}