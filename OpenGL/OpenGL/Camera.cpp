#include "Camera.h"

const float PI = atan(1.0)*4.0;

Camera::Camera() {
	eye = glm::vec3(0.0f, 12.0f, 15.0f);  
	view = glm::vec3(0.0f, 0.0, 0.0f);  
	up = glm::vec3(0.0f, 1.0f, 0.0f);   
	iForw = int('W'); iBack = int('S'); iLeft = int('A'); iRight = int('D');
	speed = 0.25f;
	sensitivity = 0.1f;
	active = true;
}

void Camera::setCursor() {
	RECT rect;
	GetWindowRect(appMain.hWnd, &rect);
	int centX = (rect.left + rect.right) / 2;
	int centY = (rect.top + rect.bottom) / 2;
	SetCursorPos(centX, centY);
}

pair<int, int> Camera::getCursorPosition() {
	return pair<int, int>(appMain.getWidth() / 2, appMain.getHeight() / 2);
}

void Camera::rotate() {
	GetCursorPos(&cursor);
	RECT rect; 
	GetWindowRect(appMain.hWnd, &rect);
	int centX = (rect.left + rect.right) / 2;
	int centY = (rect.top + rect.bottom) / 2;

	float deltaX = (float)(centX - cursor.x) * sensitivity;
	float deltaY = (float)(centY - cursor.y) * sensitivity;

	if (deltaX != 0.0f)
	{
		view -= eye;
		view = glm::rotate(view, deltaX, glm::vec3(0.0f, 1.0f, 0.0f));
		view += eye;
	}

	if (deltaY != 0.0f)
	{
		glm::vec3 axis = glm::cross(view - eye, up);
		axis = glm::normalize(axis);
		float angle = deltaY;
		float newAngle = angle + getAngleX();
		if (newAngle > -89.80f && newAngle < 89.80f)
		{
			view -= eye;
			view = glm::rotate(view, deltaY, axis);
			view += eye;
		}
	}

	SetCursorPos(centX, centY);  // back to the center
}

void Camera::move(bool considerY) {
	glm::vec3 viewY = view; if(!considerY) viewY.y = 0;
	glm::vec3 eyeY = eye;   if(!considerY) eyeY.y = 0;

	// forward and back
	glm::vec3 move = viewY - eyeY;
	move = glm::normalize(move);
	move *= speed;

	// left and right
	glm::vec3 strafe = glm::cross(viewY - eyeY, up);
	strafe = glm::normalize(strafe);
	strafe *= speed;

	glm::vec3 movement;

	if (Key::wasPressed(iForw)) movement += move;
	if (Key::wasPressed(iBack)) movement -= move;
	if (Key::wasPressed(iLeft)) movement -= strafe;
	if (Key::wasPressed(iRight)) movement += strafe;

	eye += movement;
	view += movement;
}

float Camera::getAngleY() // from 0.0f to 360.0f
{
	glm::vec3 dir = view - eye;
	dir = glm::normalize(dir);

	glm::vec3 dir2 = dir;
	dir2.x = 0.0f;

	float angle = acos(glm::dot(dir2, dir)) * (180.0f / PI);
	if (dir.z < 0.0f) {
		angle += (90.0f - angle) * 2;
	}
	if (dir.x > 0.0f) {
		angle = 360.0f - angle;
	}

	return angle;
}

float Camera::getAngleX()  // from -90.0f to 90.0f
{
	glm::vec3 dir = view - eye;
	dir = glm::normalize(dir);

	glm::vec3 dir2 = dir; 
	dir2.y = 0.0f;
	dir2 = glm::normalize(dir2);

	float angle = acos(glm::dot(dir2, dir)) * (180.0f / PI);
	if (dir.y < 0) angle *= -1.0f;

	return angle;
}

void Camera::update(bool considerY) {
	if (active) {
		rotate();
		move(considerY);
	}
}

void Camera::setHeight(float height) {
	eye.y = height;
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(eye, view, up);
}

glm::vec3 Camera::getPosition() {
	return eye;
}

glm::vec3 Camera::getDirection() {
	return eye - view;
}


glm::vec3 Camera::getUpVector() {
	return up;
}

void Camera::setActive(bool active) {
	this->active = active;
}