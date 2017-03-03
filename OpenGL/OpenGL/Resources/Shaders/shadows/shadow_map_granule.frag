#version 330 

void main()
{     
	float depthValue = gl_FragCoord.z;
    gl_FragColor = vec4(vec3(depthValue), 1.0);
} 