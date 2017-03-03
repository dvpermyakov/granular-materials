#pragma once

#include "GLControl.h"

#include <AntTweakBar.h>
#include <windows.h>
#include <commctrl.h>
#include <string>

using namespace std;

class Key {
public:
	static bool wasPressed(int virtualKey);
	static bool wasOnePressed(int virtualKey);
private:
	static bool states[256];
};

class WindowApp {
public:
	HWND hWnd;    // Handle to application window
	HWND hWndPB;  // Handle to progress bar window
	GLControl glControl;
	bool hideCursor;

	bool InitializeApp(string sAppName);
	void RegisterAppClass(HINSTANCE a_hInstance);
	bool CreateAppWindow(string title);

	int getHeight();
	int getWidth();
	string openFileDialog();
	string saveFileDialog();
	
	void createProgressBar(int size);
	void stepProgressBar();
	void removeProgressBar();

	void AppBody();
	void Shutdown();

	HINSTANCE GetInstance();

	LRESULT CALLBACK MsgHandlerMain(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE hInstance; // Application's instance
	string appName;
	HANDLE hMutex;

	bool isAppActive; // To check if application is active (not minimized)
	DWORD dwLastFrame;
	float fFrameInterval;
};

void InitScene(LPVOID), RenderScene(LPVOID), ReleaseScene(LPVOID);

extern WindowApp appMain;