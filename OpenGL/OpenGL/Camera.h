#pragma once

#include "WindowApp.h"

#include <Windows.h>
#include <utility>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace std;

class Camera {
public:
	Camera();
	void setCursor();
	pair<int, int> getCursorPosition();
	void update(bool considerY = true);
	void setHeight(float height);
	glm::mat4 getViewMatrix();
	glm::vec3 getPosition();
	glm::vec3 getDirection();
	glm::vec3 getUpVector();
	float getAngleX();
	float getAngleY();
	void setActive(bool active);
private:
	void rotate();
	void move(bool considerY);
	POINT cursor;             
	glm::vec3 eye, view, up;  
	float speed;             
	float sensitivity;        
	int iForw, iBack, iLeft, iRight;  
	bool active;
};