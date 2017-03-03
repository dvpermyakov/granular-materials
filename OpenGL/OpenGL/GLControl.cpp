#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "assimp.lib") 
#pragma comment(lib, "FreeImage.lib")
#pragma comment(lib, "freetype.lib") 
#pragma comment(lib, "irrKlang.lib") 
#pragma comment(lib, "AntTweakBar.lib")
#pragma comment(lib, "BulletCollision_Debug.lib")
#pragma comment(lib, "BulletDynamics_Debug.lib")
#pragma comment(lib, "LinearMath_Debug.lib")
#pragma comment(lib, "libboost_serialization-vc120-mt-gd-1_60.lib")
#pragma comment(lib, "comctl32.lib")

#include "GLControl.h"

#include <windows.h>
#include <sstream>
#include <GL/glew.h>
#include <gl/wglew.h>

bool    GLControl::isClassRegistered = false;
bool    GLControl::isGlewInitialized = false;
int     GLControl::fps = 0;
int     GLControl::currentFramesCount = 0;
clock_t GLControl::lastFpsSaving = 0;

//Creates fake window and OpenGL rendering context, within which GLEW is initialized.
bool GLControl::InitGLEW(HINSTANCE hInstance)
{
	if (isGlewInitialized) return true;

	RegisterSimpleOpenGLClass(hInstance);

	HWND hWndFake = CreateWindow(SIMPLE_OPENGL_CLASS_NAME, "FAKE", WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_CLIPCHILDREN,
		0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
		NULL, hInstance, NULL);

	hDC = GetDC(hWndFake);

	// First, choose false pixel format
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int iPixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (iPixelFormat == 0) return false;

	if (!SetPixelFormat(hDC, iPixelFormat, &pfd)) return false;

	// Create the false, old style context (OpenGL 2.1 and before)
	HGLRC hRCFake = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRCFake);

	bool result = true;

	if (!isGlewInitialized) {
		if (glewInit() != GLEW_OK) {
			MessageBox(*hWnd, "Couldn't initialize GLEW!", "Fatal Error", MB_ICONERROR);
			result = false;
		}
		isGlewInitialized = true;
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRCFake);
	DestroyWindow(hWndFake);

	return result;
}

// Initializes OpenGL rendering context. If succeeds, returns true.
// hInstance - application instance
// a_hWnd - window to init OpenGL into
// a_initScene - pointer to init function
// a_renderScene - pointer to render function
// a_releaseScene - optional parameter of release function
bool GLControl::InitOpenGL(HINSTANCE hInstance, HWND* a_hWnd, void(*a_InitScene)(LPVOID), void(*a_RenderScene)(LPVOID), void(*a_ReleaseScene)(LPVOID), LPVOID lpParam)
{
	if (!InitGLEW(hInstance)) return false;

	hWnd = a_hWnd;
	hDC = GetDC(*hWnd);

	bool isError = false;
	PIXELFORMATDESCRIPTOR pfd;

	// if we have access to these functions
	if (WGLEW_ARB_create_context && WGLEW_ARB_pixel_format) {
		const int iPixelFormatAttribList[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE, // Enable OpenGL support
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,  // and double buffer
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24, // Depth buffer size
			WGL_STENCIL_BITS_ARB, 8,
			0 // End of attributes list
		};
		int iContextAttribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4, // OpenGL 
			WGL_CONTEXT_MINOR_VERSION_ARB, 1, // version 4.1
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0 // End of attributes list
		};

		int iPixelFormat, iNumFormats;
		wglChoosePixelFormatARB(hDC, iPixelFormatAttribList, NULL, 1, &iPixelFormat, (UINT*)&iNumFormats);

		// PFD seems to be only redundant parameter now
		if (!SetPixelFormat(hDC, iPixelFormat, &pfd))return false;

		hRC = wglCreateContextAttribsARB(hDC, 0, iContextAttribs);
		// If everything went OK
		if (hRC) wglMakeCurrent(hDC, hRC);
		else isError = true;

	}
	else isError = true;

	if (isError) {
		// Generate error messages
		char sErrorMessage[255], sErrorTitle[255];
		sprintf(sErrorMessage, "OpenGL 4.1 is not supported! Please download latest GPU drivers!");
		sprintf(sErrorTitle, "OpenGL 4.1 Not Supported");
		MessageBox(*hWnd, sErrorMessage, sErrorTitle, MB_ICONINFORMATION);
		return false;
	}

	RenderScene = a_RenderScene;
	InitScene = a_InitScene;
	ReleaseScene = a_ReleaseScene;
	if (InitScene != NULL) InitScene(lpParam);

	return true;
}

// Resizes viewport to full window with perspective projection.
void GLControl::ResizeOpenGLViewportFull() {
	if (hWnd == NULL) return;
	RECT rRect; 
	GetClientRect(*hWnd, &rRect);
	glViewport(0, 0, rRect.right, rRect.bottom);
}

int GLControl::getViewPortHeight() {
	if (hWnd == NULL) return 0;
	RECT rRect;
	GetClientRect(*hWnd, &rRect);
	return rRect.bottom;
}

int GLControl::getViewPortWidth() {
	if (hWnd == NULL) return 0;
	RECT rRect;
	GetClientRect(*hWnd, &rRect);
	return rRect.right;
}

// Handles messages from windows that use simple OpenGL class.
LRESULT CALLBACK MsgHandlerSimpleOpenGLClass(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	switch (uiMsg) {
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	default:
		return DefWindowProc(hWnd, uiMsg, wParam, lParam); // Default window procedure
	}
	return 0;
}

// Registers simple OpenGL window class.
void GLControl::RegisterSimpleOpenGLClass(HINSTANCE hInstance)
{
	if (isClassRegistered) return;
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc = (WNDPROC) MsgHandlerSimpleOpenGLClass;
	wc.cbClsExtra = 0; wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_MENUBAR + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = SIMPLE_OPENGL_CLASS_NAME;

	RegisterClassEx(&wc);

	isClassRegistered = true;
}

void GLControl::setProjectionMatrix(float fFOV, float fAspectRatio, float fNear, float fFar) {
	projectionMatrix = glm::perspective(fFOV, fAspectRatio, fNear, fFar);
}

glm::mat4* GLControl::getProjectionMatrix() {
	return &projectionMatrix;
}

void GLControl::setOrthoMatrix(int width, int height) {
	orthoMatrix = glm::ortho(0.0f, float(width), 0.0f, float(height));
}

glm::mat4* GLControl::getOrthoMatrix() {
	return &orthoMatrix;
}

// Unregisters simple OpenGL window class.
void GLControl::UnregisterSimpleOpenGLClass(HINSTANCE hInstance) {
	if (isClassRegistered) {
		UnregisterClass(SIMPLE_OPENGL_CLASS_NAME, hInstance);
		isClassRegistered = false;
	}
}

//Swaps back and front buffer.
void GLControl::SwapBuffersM() {
	SwapBuffers(hDC);
}


// Makes current device and OpenGL rendering context to those associated with OpenGL control.
void GLControl::MakeCurrent() {
	wglMakeCurrent(hDC, hRC);
}

void GLControl::MakeCurrentNull() {
	wglMakeCurrent(NULL, NULL);
}

// Calls previously set render function.
// lpParam - pointer to whatever you want
void GLControl::Render(LPVOID lpParam) {
	clock_t time = clock();
	if (time - lastFpsSaving > CLOCKS_PER_SEC) {
		lastFpsSaving = time;
		fps = currentFramesCount;
		currentFramesCount = 0;
	}
	if (RenderScene) {
		RenderScene(lpParam);
		currentFramesCount++;
	}
}

int GLControl::getFps() {
	return fps;
}

// Calls previously set release function and deletes rendering context.
// lpParam - pointer to whatever you want
void GLControl::ReleaseOpenGLControl(LPVOID lpParam)
{
	if (ReleaseScene) ReleaseScene(lpParam);

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(*hWnd, hDC);

	hWnd = NULL;
}