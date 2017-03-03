#include "Screen.h"

const glm::vec3 color = glm::vec3(1, 1, 1);

void SplashScreen::setFont(string name, int size) {
	font.loadSystemFont(name, size);
}

void SplashScreen::setGLControl(GLControl* control) {
	this->control = control;
}

void SplashScreen::render(string forRendering) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	string text = "Loading of " + forRendering + "...";
	int textLength = font.getLength(text);
	int width = (control->getViewPortWidth() - textLength) / 2;
	int height = control->getViewPortHeight() / 2;
	font.print(*control->getOrthoMatrix(), text, color, width, height);
	control->SwapBuffersM();
}