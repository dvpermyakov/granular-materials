#include "WindowApp.h"

using namespace std;

WindowApp appMain;

bool Key::states[256] = { false };

bool Key::wasPressed(int virtualKey) {
	SHORT keyState = GetAsyncKeyState(virtualKey);
	bool pressed = (1 << 16) & keyState;
	states[virtualKey] = pressed;
	return pressed;
}

bool Key::wasOnePressed(int virtualKey) {
	SHORT keyState = GetAsyncKeyState(virtualKey);
	bool pressed = (1 << 16) & keyState;
	if (pressed && !states[virtualKey]) {
		states[virtualKey] = pressed;
		return true;
	} else {
		states[virtualKey] = pressed;
		return false;
	}
}

// Initializes app with specified (unique) application identifier.
bool WindowApp::InitializeApp(string sAppName) {
	appName = sAppName;
	hMutex = CreateMutex(NULL, 1, appName.c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, "This application already runs!", "Multiple Instances Found.", MB_ICONINFORMATION | MB_OK);
		return false;
	}
	hideCursor = true;
	return true;
}

// Registers application window class.
// a_hInstance - application instance handle
LRESULT CALLBACK GlobalMessageHandler(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
	return appMain.MsgHandlerMain(hWnd, uiMsg, wParam, lParam);
}

void WindowApp::RegisterAppClass(HINSTANCE a_hInstance) {
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;

	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	wcex.hIcon = LoadIcon(hInstance, IDI_WINLOGO);
	wcex.hIconSm = LoadIcon(hInstance, IDI_WINLOGO);
	wcex.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wcex.hInstance = hInstance;

	wcex.lpfnWndProc = GlobalMessageHandler;  // use callback in above
	wcex.lpszClassName = appName.c_str();

	wcex.lpszMenuName = NULL;

	RegisterClassEx(&wcex);
}

// Creates main application window.
bool WindowApp::CreateAppWindow(string title) {
	DEVMODE dmSettings;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmSettings);
	hWnd = CreateWindowEx(0, appName.c_str(), title.c_str(), WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, dmSettings.dmPelsWidth, dmSettings.dmPelsHeight,
		NULL, NULL, hInstance, NULL);

	if (!glControl.InitOpenGL(hInstance, &hWnd, InitScene, RenderScene, NULL, &glControl)) return false;

	ShowWindow(hWnd, SW_SHOWMAXIMIZED);
	UpdateWindow(hWnd);

	return true;
}

// Main application body infinite loop.
void WindowApp::AppBody() {
	MSG msg;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break; // If the message was WM_QUIT then exit application
			else {
				TranslateMessage(&msg); // Otherwise send message to appropriate window
				DispatchMessage(&msg);
			}
		}
		else if (isAppActive) {
			glControl.Render(&glControl);
		}
		else Sleep(200); // Do not consume processor power if application isn't active
	}
}

// Shuts down application and releases used memory.
void WindowApp::Shutdown() {
	glControl.ReleaseOpenGLControl(&glControl);

	DestroyWindow(hWnd);
	UnregisterClass(appName.c_str(), hInstance);
	GLControl::UnregisterSimpleOpenGLClass(hInstance);
	ReleaseMutex(hMutex);
}

// Application messages handler.
LRESULT CALLBACK WindowApp::MsgHandlerMain(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
	if (TwEventWin(hWnd, uiMsg, wParam, lParam)) return 0;  // it is used in ATBInterface

	PAINTSTRUCT ps;
	switch (uiMsg) {
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_ACTIVATE: {
		switch (LOWORD(wParam)) {
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			isAppActive = true;
			break;
		case WA_INACTIVE:
			isAppActive = false;
			break;
		}
		break;
	}

	case WM_SETCURSOR:
		if(LOWORD(lParam) == HTCLIENT) {
			if (hideCursor) SetCursor(NULL);
			else SetCursor(LoadCursor(hInstance, IDC_ARROW));
			return TRUE;
		}
		break;

	case WM_SIZE:
		glControl.ResizeOpenGLViewportFull();
		glControl.setProjectionMatrix(45.0f, float(LOWORD(lParam)) / float(HIWORD(lParam)), 0.05f, 1000.0f);
		glControl.setOrthoMatrix(LOWORD(lParam), HIWORD(lParam));
		break;

	default:
		return DefWindowProc(hWnd, uiMsg, wParam, lParam);
	}
	return 0;
}

int WindowApp::getHeight() {
	return glControl.getViewPortHeight();
}

int WindowApp::getWidth() {
	return glControl.getViewPortWidth();
}

string WindowApp::openFileDialog() {
	OPENFILENAME ofn;
	char name[260];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = name;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(name);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn) == TRUE) return ofn.lpstrFile;
	else return "";
}

string WindowApp::saveFileDialog() {
	OPENFILENAME ofn;
	char name[260];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = name;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(name);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetSaveFileName(&ofn) == TRUE) return ofn.lpstrFile;
	else return "";
}

void WindowApp::createProgressBar(int size) {
	RECT rect;
	int cyVScroll;

	InitCommonControls();
	GetClientRect(hWnd, &rect);
	cyVScroll = GetSystemMetrics(SM_CYVSCROLL);

	hWndPB = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL,
		WS_CHILD | WS_VISIBLE, rect.left,
		rect.bottom - cyVScroll,
		rect.right, cyVScroll,
		hWnd, (HMENU)0, hInstance, NULL);

	SendMessage(hWndPB, PBM_SETRANGE, 0, MAKELPARAM(0, size));
	SendMessage(hWndPB, PBM_SETSTEP, (WPARAM)1, 0);
}

void WindowApp::stepProgressBar() {
	SendMessage(hWndPB, PBM_STEPIT, 0, 0);
}

void WindowApp::removeProgressBar() {
	DestroyWindow(hWndPB);
}

// Returns application instance.
HINSTANCE WindowApp::GetInstance() {
	return hInstance;
}

//Windows entry point for application.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow) {
	srand(time(NULL));

	if (!appMain.InitializeApp("Permyakov D.V.")) return 0;

	appMain.RegisterAppClass(hInstance);

	if (!appMain.CreateAppWindow("Permyakov D.V.")) return 0;

	appMain.AppBody();

	appMain.Shutdown();

	return 0;
}