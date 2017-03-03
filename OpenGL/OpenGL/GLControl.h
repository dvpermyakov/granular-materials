#pragma once

#include <windows.h>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>

#define SIMPLE_OPENGL_CLASS_NAME "Simple_openGL_class"

using namespace std;

class GLControl {
public:
	static void RegisterSimpleOpenGLClass(HINSTANCE hInstance);
	static void UnregisterSimpleOpenGLClass(HINSTANCE hInstance);

	bool InitOpenGL(HINSTANCE hInstance, HWND* a_hWnd, void(*a_InitScene)(LPVOID), void(*a_RenderScene)(LPVOID), void(*a_ReleaseScene)(LPVOID), LPVOID lpParam);

	void ResizeOpenGLViewportFull();

	int getViewPortHeight();
	int getViewPortWidth();

	void setProjectionMatrix(float fFOV, float fAspectRatio, float fNear, float fFar);
	void setOrthoMatrix(int width, int height);
	glm::mat4* getProjectionMatrix();
	glm::mat4* getOrthoMatrix();

	void Render(LPVOID lpParam);
	void ReleaseOpenGLControl(LPVOID lpParam);

	int getFps();

	void MakeCurrent();
	void MakeCurrentNull();
	void SwapBuffersM();

private:
	bool InitGLEW(HINSTANCE hInstance);

	HDC hDC;
	HWND* hWnd;
	HGLRC hRC;
	glm::mat4 projectionMatrix;
	glm::mat4 orthoMatrix;

	static bool isClassRegistered;
	static bool isGlewInitialized;

	static int fps;                 // it is used for store frames per last second
	static int currentFramesCount;  // it is used for store frames in current second
	static clock_t lastFpsSaving;   // last time when we saved fps

	void(*InitScene)(LPVOID lpParam), (*RenderScene)(LPVOID lpParam), (*ReleaseScene)(LPVOID lpParam);
};

LRESULT CALLBACK MsgHandlerSimpleOpenGLClass(HWND, UINT, WPARAM, LPARAM);