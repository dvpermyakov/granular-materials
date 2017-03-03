#version 330

const int MAX_POINT_LIGHTS = 10;

struct Camera {
	vec3 position;
};

struct DirectLight{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	bool enable;
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	float constant;
    float linear;
    float quadratic; 
	float farPlane;
	float nearPlane;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

uniform Camera camera;
uniform DirectLight light;
uniform PointLight pointLight[MAX_POINT_LIGHTS];
uniform Material material;
uniform sampler2D shadowMap;
uniform samplerCube cubeShadowMap[MAX_POINT_LIGHTS];
uniform bool withShadow;
uniform int pointLightCount;
uniform bool onlyAmbient;

in vec2 theTexCoord;
in vec3 theNormal;
in vec4 thePosition;
in vec4 theWorldPosition;
in vec4 theLightPosition;

float getPointLightDegree(vec4 theWorldPosition, PointLight pointLight) {
	float distance = length(pointLight.position - vec3(theWorldPosition));
	return 1.0f / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * pow(distance, 2));  
}

float getDiffuseDegree(vec3 theNormal, DirectLight light) {
	return normalize(max(dot(theNormal, normalize(light.direction)), 0.0));
}

float getSpecularDegree(vec4 theWorldPosition, vec3 theNormal, DirectLight light, Camera camera, Material material) {
	vec3 direction = normalize(reflect(-light.direction, theNormal));
	vec3 viewDirection = normalize(camera.position - vec3(theWorldPosition));
	if (material.shininess > 0.0) {
		return pow(max(dot(viewDirection, direction), 0.0), material.shininess);
	} else {
		return 0.0;
	}
}

float getShadowDegree(vec4 theLightPosition, sampler2D shadowMap, DirectLight light) {
	vec3 position = theLightPosition.xyz / theLightPosition.w;
	position = position * 0.5 + 0.5;
	float currentDepth = position.z;
	if (currentDepth > 1.0) return 1.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	float shadow = 0;
	for (int x = -2; x <= 2; x++) {
		for (int y = -2; y <= 2; y++) {
			float lightDepth = texture(shadowMap, position.xy + vec2(x, y) * texelSize).r;
			float bias = max(0.01 * (1.0 - dot(theNormal, normalize(light.direction))), 0.005);  
			shadow += currentDepth - lightDepth > bias ? 1.0 : 0.0;
		}
	}
	return shadow / 25.0;
}

vec3 sampleOffsetDirections[20] = vec3[] (
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float getCubeShadowDegree(PointLight light, vec4 theWorldPosition, samplerCube cubeShadowMap) {
	vec3 position = vec3(theWorldPosition) - light.position;
	float currentDepth = length(position);
	if (currentDepth > light.farPlane || currentDepth < light.nearPlane) return 0.0;
	float shadow = 0;
	float diskRadius = (1.0 + (length(camera.position - vec3(theWorldPosition)) / light.farPlane)) / 25.0; 
	for (int i = 0; i < 20; i++) {
		float lightDepth = texture(cubeShadowMap, position + sampleOffsetDirections[i] * diskRadius).r * light.farPlane;
		shadow += currentDepth - lightDepth > 0.7 ? 1.0 : 0.0;
	}
	return shadow / 20;
}

void main() {
	vec3 finalColor = vec3(texture2D(material.diffuse, theTexCoord));
	float shadow = withShadow ? getShadowDegree(theLightPosition, shadowMap, light) : 0.0;
	vec3 ambientColor = light.ambient * finalColor;
    vec3 diffuseColor = light.direction.y < 0 ? vec3(0, 0, 0) : (1.0 - shadow) * light.diffuse * getDiffuseDegree(theNormal, light) * finalColor;
	vec3 specularColor = light.direction.y < 0 ? vec3(0, 0, 0) : (1.0 - shadow) * light.specular *  getSpecularDegree(theWorldPosition, theNormal, light, camera, material) * finalColor;
	//vec3 pointColor = vec3(0, 0, 0);
	//for (int i = 0; i < pointLightCount; i++) {
	//	if (!pointLight[i].enable) continue;
	//	float cubeShadow = 0.0;
	//	if (i == 0) cubeShadow = getCubeShadowDegree(pointLight[i], theWorldPosition, cubeShadowMap[0]);
	//	if (i == 1) cubeShadow = getCubeShadowDegree(pointLight[i], theWorldPosition, cubeShadowMap[1]);
	//	float lightDegree = getPointLightDegree(theWorldPosition, pointLight[i]);
	//	pointColor += lightDegree * pointLight[i].ambient * finalColor;
	//	pointColor += (1.0 - cubeShadow) * lightDegree * pointLight[i].diffuse * finalColor;
	//}
	if (!onlyAmbient) {
		gl_FragColor = vec4(vec3(ambientColor + diffuseColor + specularColor), 1);
		//gl_FragColor = vec4(ambientColor * 2, 1);
	} 
	else {
		gl_FragColor = vec4(ambientColor, 1);
	}
}