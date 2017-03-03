#version 330

uniform struct Matrices {
	mat4 projection;
	mat4 view;
} matrices;

uniform vec3 quad1, quad2;

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

in vec3 theColor[];
in float theLifeTime[];
in float theSize[];
in int theType[];

smooth out vec2 texCoord;
flat out vec4 colorPart;

void main() {
	if(theType[0] != 0) {
		vec3 oldPosition = gl_in[0].gl_Position.xyz;
		float size = theSize[0];
		mat4 mat = matrices.projection * matrices.view;
		colorPart = vec4(theColor[0], theLifeTime[0]);	
		
		// 1st
		vec3 position = oldPosition + (-quad1 - quad2) * size;
		texCoord = vec2(0.0, 0.0);
		gl_Position = mat * vec4(position, 1.0);
		EmitVertex();

		// 2nd
		position = oldPosition + (-quad1 + quad2) * size;
		texCoord = vec2(0.0, 1.0);
		gl_Position = mat * vec4(position, 1.0);
		EmitVertex();
		
		// 3td
		position = oldPosition + (quad1-quad2) * size;
		texCoord = vec2(1.0, 0.0);
		gl_Position = mat * vec4(position, 1.0);
		EmitVertex();
		
		// 4th
		position = oldPosition + (quad1 + quad2) * size;
		texCoord = vec2(1.0, 1.0);
		gl_Position = mat * vec4(position, 1.0);
		EmitVertex();
      
		EndPrimitive();
	}
}