#pragma once

#include "Text.h"
#include "GLControl.h"

#include <GL/glew.h>
#include <gl/wglew.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SplashScreen {
public:
	void setGLControl(GLControl* control);
	void setFont(string name, int size);
	void render(string forRendering);
private:
	GLFont font;
	GLControl* control;
};

class MenuScreen {
public:
	void setGLControl(GLControl* control);
	void render();
private:
	GLControl* control;
};