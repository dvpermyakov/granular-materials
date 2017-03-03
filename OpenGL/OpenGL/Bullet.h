#pragma once

#include "Environment.h"
#include "Camera.h"
#include "Resources.h"
#include "Audio.h"

class Bullet : public EnvironmentModel {
public:
	static void loadModel();
	Bullet(Camera camera);
	glm::mat4 getModelMatrix();
	bool isAlive();
	glm::vec3 getVelocity();
	void contactAction();
	void remove();
private:
	static Model bulletModel;
	Model* getModel();
	float maxLength;
	glm::vec3 direction;
	glm::vec3 initialPosition;
	float angleX;
	float angleY;
	float speed;
};