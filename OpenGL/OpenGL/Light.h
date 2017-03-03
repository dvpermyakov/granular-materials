#pragma once

#include "Model.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

class DirectLight {
public:
	DirectLight();
	glm::vec3 getDirection();
	glm::vec3 getPosition();
	glm::vec3 getAmbientColor();
	glm::vec3 getDiffuseColor();
	glm::vec3 getSpecularColor();
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	glm::mat4 getLargeProjectionMatrix();
	void update();
	void moveUp();
	void moveDown();
	float getAngle();
	float getIntensity();
	float getDistance();
private:
	float rotationAngle;
	float minAngle;
	glm::vec3 eye;
	glm::vec3 view;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

enum Distance {
	DISTANCE_20 = 0,
	DISTANCE_32,
	DISTANCE_50,
	DISTANCE_65,
};

class PointLight {
public:
	PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, Distance distance);
	glm::vec3 getPosition();
	glm::vec3 getAmbient();
	glm::vec3 getDiffuse();
	float getConstant();
	float getLinear();
	float getQuadratic();
	float getFarPlane();
	float getNearPlane();
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix(GLenum type);
	void switchLight();
	bool isEnable();
private:
	glm::vec3 position;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	float constant;
	float linear;
	float quadratic;
	float farPlane;
	float nearPlane;
	bool enable;
};