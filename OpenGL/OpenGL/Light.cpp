#include "Light.h"
#include "WindowApp.h"

const float PI = 3.14159265358979;

DirectLight::DirectLight() {
	//eye = glm::vec3(0.0f, 10.0f, 0.0f);
	//view = glm::vec3(0.0f);
	//eye =  glm::vec3(0.4f, 14.34f, 1.4f) * 0.5f;
	//view = glm::vec3(0.4f, 0.66f, -0.1f);
	eye = glm::vec3(0.0f, 4.0f, 0.0f);
	view = glm::vec3(0.0f);

	ambient = glm::vec3(0.2f); 
	diffuse = glm::vec3(0.5f);
	specular = glm::vec3(0.7f);

	rotationAngle = 10.0f / 180.0f * PI;
	minAngle = 10.0f;
}

glm::vec3 DirectLight::getPosition() {
	return eye;
}


glm::vec3 DirectLight::getDirection() {
	return eye - view;
}

glm::vec3 DirectLight::getAmbientColor() {
	return ambient;
}

glm::vec3 DirectLight::getDiffuseColor() {
	return diffuse;
}

glm::vec3 DirectLight::getSpecularColor() {
	return specular;
}

glm::mat4  DirectLight::getViewMatrix() {
	return glm::lookAt(eye, view, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 DirectLight::getProjectionMatrix() {
	return glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.01f, 6.0f);
}

glm::mat4 DirectLight::getLargeProjectionMatrix() {
	return glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.0001f, 300.0f);
}

void DirectLight::update() {
	if (getAngle() > 180.0f) {
		eye -= view;
		eye = glm::rotate(eye, rotationAngle *  10, glm::vec3(1, 0, 0));
		eye += view;
	}
	else {
		eye -= view;
		eye = glm::rotate(eye, rotationAngle, glm::vec3(1, 0, 0));
		eye += view;
	}
}

float DirectLight::getAngle() {
	glm::vec3 direction = glm::normalize(getDirection());
	glm::vec3 yDirection = direction;
	yDirection.y = 0;
	float angle = acos(glm::dot(yDirection, direction)) * (180.0f / PI);
	if (direction.z < 0.0f) {
		angle += (90.0f - angle) * 2;
	}
	if (direction.y < 0.0f) {
		angle = 360.0f - angle;
	}
	return angle;
}

void DirectLight::moveUp() {
	eye -= view;
	eye *= 1.1;
	eye += view;
}

void DirectLight::moveDown() {
	eye -= view;
	eye *= 0.9;
	eye += view;
}

float DirectLight::getDistance() {
	return glm::length(eye - view);
}

float DirectLight::getIntensity() {
	float angle = getAngle();
	if (angle >= 180.0f) {
		return 0.0;
	}
	if (angle >= 90.0f) {
		return 1.0 - (angle - 90.0f) / 90.0f;
	}
	return angle / 90.0f;
}

// Point light

PointLight::PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, Distance distance) : position(position), ambient(ambient), diffuse(diffuse), 
																									  constant(1.0f), nearPlane(3.0f), enable(false) {
	switch (distance) {
	case DISTANCE_20:
		linear = 0.22f; quadratic = 0.20f; farPlane = 20.0f;
		break;
	case DISTANCE_32:
		linear = 0.14f; quadratic = 0.07f; farPlane = 32.0f;
		break;
	case DISTANCE_50:
		linear = 0.09f; quadratic = 0.032f; farPlane = 50.0f;
		break;
	case DISTANCE_65:
		linear = 0.07f; quadratic = 0.017f; farPlane = 65.0f;
		break;
	}
}

glm::vec3 PointLight::getPosition() {
	return position;
}

glm::vec3 PointLight::getAmbient() {
	return ambient;
}

glm::vec3 PointLight::getDiffuse() {
	return diffuse;
}

float PointLight::getConstant() {
	return constant;
}

float PointLight::getLinear() {
	return linear;
}

float PointLight::getQuadratic() {
	return quadratic;
}

glm::mat4 PointLight::getViewMatrix(GLenum type) {
	switch (type) {
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
		return glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
		return glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
		return glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
		return glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
		return glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
		return glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
	}
}

glm::mat4 PointLight::getProjectionMatrix() {
	return glm::perspective(90.0f, 1.0f, 0.1f, farPlane);
}

float PointLight::getFarPlane() {
	return farPlane;
}

float PointLight::getNearPlane() {
	return nearPlane;
}

void PointLight::switchLight() {
	enable = !enable;
}

bool PointLight::isEnable() {
	return enable;
}