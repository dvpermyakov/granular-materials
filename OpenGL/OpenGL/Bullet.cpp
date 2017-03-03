#include "Bullet.h"

const float PI = atan(1.0)*4.0;

Model Bullet::bulletModel;

void Bullet::loadModel() {
	bulletModel.loadModelFromFile(BULLET_MODEL_LOCATION);
	bulletModel.generateVao();
}

Bullet::Bullet(Camera camera) : EnvironmentModel(), maxLength(100.0f), speed(100.0f) {
	this->direction = glm::normalize(-camera.getDirection());
	this->position = camera.getPosition();
	this->initialPosition = this->position;
	this->scale = glm::vec3(0.01f);
	angleX = -camera.getAngleX();
	angleY = -camera.getAngleY();
}

Model* Bullet::getModel() {
	return &bulletModel;
}

glm::mat4 Bullet::getModelMatrix() {
	glm::mat4 modelMatrix(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::rotate(modelMatrix, angleY, glm::vec3(0, 1, 0));
	modelMatrix = glm::rotate(modelMatrix, angleX, glm::vec3(1, 0, 0));
	modelMatrix = glm::scale(modelMatrix, scale);
	return modelMatrix;
}

bool Bullet::isAlive() {
	return glm::length(this->position - this->initialPosition) < maxLength;
}

void Bullet::remove() {
	this->position.x += this->initialPosition.x + maxLength + 1;
}

glm::vec3 Bullet::getVelocity() {
	return speed * direction;
}

void Bullet::contactAction() {
	sound.play2D(SOUND_COLLISION_BULLET);
}