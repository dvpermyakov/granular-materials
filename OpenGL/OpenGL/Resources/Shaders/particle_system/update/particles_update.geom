#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 40) out;

in vec3 thePosition[];
in vec3 theVelocity[];
in vec3 theColor[];
in float theLifeTime[];
in float theSize[];
in int theType[];

out vec3 positionOut;
out vec3 velocityOut;
out vec3 colorOut;
out float lifeTimeOut;
out float sizeOut;
out int typeOut;

struct ParticleSettings {
	vec3 position;
	vec3 velocityMin;
	vec3 velocityRange;
	vec3 gravity;
	vec3 color;
	float lifeTimeMin;
	float lifeTimeRange;
	float size;
	int generateParticleAmount;
};

uniform ParticleSettings settings;
uniform float timePassed;
uniform vec3 randomSeed;
vec3 localSeed;

float randZeroOne() {
    uint n = floatBitsToUint(localSeed.y * 214013.0 + localSeed.x * 2531011.0 + localSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    localSeed = vec3(localSeed.x + 147158.0 * fRes, localSeed.y * fRes  + 415161.0 * fRes, localSeed.z + 324154.0 * fRes);
    return fRes;
}

void main()
{
	localSeed = randomSeed;

	positionOut = thePosition[0];
	velocityOut = theVelocity[0];
	if(theType[0] != 0) positionOut += velocityOut * timePassed / 1000.0;
	if(theType[0] != 0) velocityOut += settings.gravity * timePassed / 1000.0;

	colorOut = theColor[0];
	lifeTimeOut = theLifeTime[0] - timePassed;
	sizeOut = theSize[0];
	typeOut = theType[0];
    
	if(typeOut == 0) {
		EmitVertex();
		EndPrimitive();
    
		for(int i = 0; i < settings.generateParticleAmount; i++) {
		positionOut = settings.position;
		velocityOut = settings.velocityMin + vec3(settings.velocityRange.x * randZeroOne(), settings.velocityRange.y * randZeroOne(), settings.velocityRange.z * randZeroOne());
		colorOut = settings.color;
		lifeTimeOut = settings.lifeTimeMin + settings.lifeTimeRange * randZeroOne();
		sizeOut = settings.size;
		typeOut = 1;
		EmitVertex();
		EndPrimitive();
		}
	}
	else if(lifeTimeOut > 0.0) {
		EmitVertex();
		EndPrimitive(); 
	}
}