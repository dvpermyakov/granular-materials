#version 330

struct Camera {
	vec3 position;
};

struct DirectLight{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material {
	vec3 color;
	samplerCube normal;
    samplerCube diffuse;
    float     shininess;
};

uniform Camera camera;
uniform Material material;
uniform DirectLight light;
uniform sampler2D shadowMap;
 
in vec3 theNormal;
in vec4 theWorldPosition;
in vec4 theLightPosition;
in mat3 TBN;

float getDiffuseDegree(vec3 theNormal, DirectLight light) {
	return max(dot(theNormal, normalize(light.direction)), 0.0);
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
	if (currentDepth > 1.0) return 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	float shadow = 0;
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			float lightDepth = texture(shadowMap, position.xy + vec2(x, y) * texelSize).r;
			float bias = max(0.01 * (1.0 - dot(theNormal, normalize(light.direction))), 0.005);  
			shadow += currentDepth - lightDepth > bias ? 1.0 : 0.0;
		}
	}
	return shadow / 9.0;
}

void main() {
	//vec3 normal = normalize(normalize(texture(material.normal, theNormal).rgb * 2.0 - 1.0));
	//vec3 normal = normalize(TBN * normalize(texture(material.normal, theNormal).rgb * 2.0 - 1.0));
	vec3 normal = theNormal;
	vec4 textureColor = vec4(material.color, 1) * texture(material.diffuse, theNormal);
	float shadow = getShadowDegree(theLightPosition, shadowMap, light);
	vec3 lightColor = light.ambient + light.diffuse * (1.5 - shadow) * getDiffuseDegree(normal, light) + light.specular * (1.0 - shadow) * getSpecularDegree(theWorldPosition, normal, light, camera, material);
	//vec3 lightColor   = light.ambient * (1.0 - shadow);
	gl_FragColor = vec4(lightColor, 1) * textureColor;
}