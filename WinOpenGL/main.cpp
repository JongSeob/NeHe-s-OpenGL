#include <windows.h>                              // Header File For Windows

#include <gl.h>                                // Header File For The OpenGL32 Library
#include <glu.h>                               // Header File For The GLu32 Library
#include <glaux.h>                             // Header File For The GLaux Library

HGLRC           hRC = NULL;			// Permanent Rendering Context. 모든 OpenGL 프로그램은 Rendering Context(RC)에 연결(Link)되어 있다.
// RC는 OpenGL이 Device Context를 호출하기위한 링크이다.
HDC             hDC = NULL;				// Private GDI Device Context(DC). 프로그램이 Window창에 그림을 그리기 위해 꼭 생성해야한다.
// DC는 Window창을 GDI(Graphic Device Interface)에 연결한다.
HWND            hWnd = NULL;				// Holds Our Window Handle. 윈도우 자체를 위한 핸들. 2개이상 존재할 수 있다.(여러개의 창을 사용하는 프로그램의 경우)
HINSTANCE       hInstance;						// Holds The Instance Of The Application. 프로그램 코드를 담고 있는 모듈에 대한 핸들.
// 프로그램 코드 덩어리를 윈도우에서 관리하기 위해서 부여한 고유 식별 번호.
// http://terapi.tistory.com/entry/HWND%EC%99%80-HINSTANCE%EC%9D%98-%EC%B0%A8%EC%9D%B4-API 참조.

bool    keys[256];						// Array Used For The Keyboard Routine
bool    active = TRUE;						// Window Active Flag Set To TRUE By Default. 윈도우가 최소화된 경우 FALSE
bool    fullscreen = TRUE;						// Fullscreen Flag. TRUE - Full Screen, FALSE - Window Mode
// 그래픽카드에따라 지원여부가 갈린다.

const int windowWidth = 1920;	// Window 창 크기
const int windowHeight = 1080;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);		// Declaration For WndProc. 윈도우 메시지 콜백함수.
// ** CreateGLWindow() 함수에서 사용하기 위해서 선언하였다.



GLvoid ReSizeGLScene(GLsizei width, GLsizei height)         // 윈도우 크기가 변경되면 변경된 크기에 맞춰 GL Window를 초기화
{
	if (height == 0)                              // Prevent A Divide By Zero By
	{
		height = 1;                           // Making Height Equal One
	}

	glViewport(0, 0, width, height);                    // Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
	glLoadIdentity();                           // Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);                 // Select The Modelview Matrix
	glLoadIdentity();                           // Reset The Modelview Matrix
}

int InitGL(GLvoid)                              // All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);                        // Enables Smooth Shading
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);                   // Black Background
	glClearDepth(1.0f);                         // Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Test To Do(Less or Equal)
	// 들어온 depth value가 depth buffer에 있는 값보다 작거나 같으면(거리가 멀거나 같으면) 통과(화면에 출력)

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	// 어떠한 연산을 할 때 드라이버에게 주는 언질 같은 것.
	// 강제하는 것이 아니기 때문에 드라이버가 무시할 수도 있다.
	// http://alleysark.tistory.com/225
	return TRUE;                                // Initialization Went OK
}

int DrawGLScene(GLvoid)                             // Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         // Clear The Screen And The Depth Buffer
	glLoadIdentity();                           // Reset The Current Modelview Matrix
	/* OpenGL 그리기 코드를 작성 */
	return TRUE;                                // Everything Went OK
}

GLvoid KillGLWindow(GLvoid)                         // Properly Kill The Window
{
	if (fullscreen)                             // Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);                  // If So Switch Back To The Desktop
		ShowCursor(TRUE);                       // Show Mouse Pointer
	}

	if (hRC)                                // Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))                 // Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))                 // Are We Able To Delete The RC?
		{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;                           // Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))                    // Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;                           // Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))                   // Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;                          // Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL", hInstance))               // Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;                         // Set hInstance To NULL
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint      PixelFormat;                        // Holds The Results After Searching For A Match
	WNDCLASS    wc;                         // Windows Class Structure
	DWORD       dwExStyle;                      // Window Extended Style
	DWORD       dwStyle;                        // Window Style

	RECT WindowRect;                            // Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;                        // Set Left Value To 0
	WindowRect.right = (long)width;                       // Set Right Value To Requested Width
	WindowRect.top = (long)0;                         // Set Top Value To 0
	WindowRect.bottom = (long)height;                     // Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;                      // Set The Global Fullscreen Flag

	hInstance = GetModuleHandle(NULL);            // Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;       // Redraw On Move, And Own DC For Window
	wc.lpfnWndProc = (WNDPROC)WndProc;                // WndProc Handles Messages
	wc.cbClsExtra = 0;                        // No Extra Window Data
	wc.cbWndExtra = 0;                        // No Extra Window Data
	wc.hInstance = hInstance;                    // Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);          // Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);          // Load The Arrow Pointer
	wc.hbrBackground = NULL;                     // No Background Required For GL
	wc.lpszMenuName = NULL;                     // We Don't Want A Menu
	wc.lpszClassName = "OpenGL";                 // Set The Class Name

	// 윈도우클래스 등록
	if (!RegisterClass(&wc))                        // Attempt To Register The Window Class
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                           // Exit And Return FALSE
	}

	if (fullscreen)                             // Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;                   // Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));       // Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);       // Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;            // Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;           // Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;             // Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Run In A Window.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;               // Select Windowed Mode (Fullscreen=FALSE)
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;                   // Exit And Return FALSE
			}
		}
	}
	if (fullscreen)                             // Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;                  // Window Extended Style
		dwStyle = WS_POPUP;                       // Windows Style
		ShowCursor(FALSE);                      // Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;           // Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;                    // Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);     // Adjust Window To True Requested Size

	if (!(hWnd = CreateWindowEx(dwExStyle,              // Extended Style For The Window
		"OpenGL",               // Class Name
		title,                  // Window Title
		WS_CLIPSIBLINGS |           // Required Window Style
		WS_CLIPCHILDREN |           // Required Window Style
		dwStyle,                // Selected Window Style
		0, 0,                   // Window Position
		WindowRect.right - WindowRect.left,   // Calculate Adjusted Window Width
		WindowRect.bottom - WindowRect.top,   // Calculate Adjusted Window Height
		NULL,                   // No Parent Window
		NULL,                   // No Menu
		hInstance,              // Instance
		NULL)))                 // Don't Pass Anything To WM_CREATE
	{
		KillGLWindow();                         // Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}
	static  PIXELFORMATDESCRIPTOR pfd =                  // pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),                  // Size Of This Pixel Format Descriptor
		1,                              // Version Number
		PFD_DRAW_TO_WINDOW |                        // Format Must Support Window
		PFD_SUPPORT_OPENGL |                        // Format Must Support OpenGL
		PFD_DOUBLEBUFFER,                       // Must Support Double Buffering
		PFD_TYPE_RGBA,                          // Request An RGBA Format
		bits,                               // Select Our Color Depth
		0, 0, 0, 0, 0, 0,                       // Color Bits Ignored
		0,                              // No Alpha Buffer
		0,                              // Shift Bit Ignored
		0,                              // No Accumulation Buffer
		0, 0, 0, 0,                         // Accumulation Bits Ignored
		16,                             // 16Bit Z-Buffer (Depth Buffer)
		0,                              // No Stencil Buffer
		0,                              // No Auxiliary Buffer
		PFD_MAIN_PLANE,                         // Main Drawing Layer
		0,                              // Reserved
		0, 0, 0                             // Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))                         // Did We Get A Device Context?
	{
		KillGLWindow();                         // Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))             // Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();                         // Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))               // Are We Able To Set The Pixel Format?
	{
		KillGLWindow();                         // Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))                   // Are We Able To Get A Rendering Context?
	{
		KillGLWindow();                         // Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))                        // Try To Activate The Rendering Context
	{
		KillGLWindow();                         // Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	ShowWindow(hWnd, SW_MAXIMIZE);                       // Show The Window
	SetForegroundWindow(hWnd);                      // Slightly Higher Priority
	SetFocus(hWnd);                             // Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);                       // Set Up Our Perspective GL Screen

	if (!InitGL())                              // Initialize Our Newly Created GL Window
	{
		KillGLWindow();                         // Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                           // Return FALSE
	}

	return TRUE;                                // Success
}
// 윈도우 메시지 콜백함수
LRESULT CALLBACK WndProc(HWND    hWnd,                   // Handle For This Window
	UINT    uMsg,                   // Message For This Window
	WPARAM  wParam,                 // Additional Message Information
	LPARAM  lParam)                 // Additional Message Information
{
	switch (uMsg)                               // Check For Windows Messages
	{
	case WM_ACTIVATE:                       // Watch For Window Activate Message
	{
		if (!HIWORD(wParam))                    // Check Minimization State
		{
			active = TRUE;                    // Program Is Active
		}
		else
		{
			active = FALSE;                   // Program Is No Longer Active
		}

		return 0;                       // Return To The Message Loop
	}

	case WM_SYSCOMMAND:                     // Intercept System Commands
	{
		switch (wParam)                     // Check System Calls
		{
		case SC_SCREENSAVE:             // Screensaver Trying To Start?
		case SC_MONITORPOWER:               // Monitor Trying To Enter Powersave?
			return 0;                   // Prevent From Happening
		}
		break;                          // Exit
	}

	case WM_CLOSE:                          // Did We Receive A Close Message?
	{
		PostQuitMessage(0);                 // Send A Quit Message
		return 0;                       // Jump Back
	}

	case WM_KEYDOWN:                        // Is A Key Being Held Down?
	{
		keys[wParam] = TRUE;                    // If So, Mark It As TRUE
		return 0;                       // Jump Back
	}

	case WM_KEYUP:                          // Has A Key Been Released?
	{
		keys[wParam] = FALSE;                   // If So, Mark It As FALSE
		return 0;                       // Jump Back
	}

	case WM_SIZE:                           // Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));       // LoWord=Width, HiWord=Height
		return 0;                       // Jump Back
	}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
// Win32 프로젝트 Main 함수
int WINAPI WinMain(HINSTANCE   hInstance,              // Instance
	HINSTANCE   hPrevInstance,              // Previous Instance
	LPSTR       lpCmdLine,              // Command Line Parameters
	int     nCmdShow)               // Window Show State
{
	MSG msg;                                // Windows Message Structure
	BOOL done = FALSE;                         // Bool Variable To Exit Loop

	// Ask The User Which Screen Mode They Prefer
	// 	if (MessageBox(NULL, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	// 	{
	// 		fullscreen = FALSE;                       // Windowed Mode
	// 	}
	fullscreen = FALSE; // 윈도우모드로 고정.

	// Create Our OpenGL Window
	if (!CreateGLWindow("NeHe's OpenGL Framework", windowWidth, windowHeight, 16, fullscreen))
	{
		return 0;                           // Quit If Window Was Not Created
	}

	while (!done)                                // Loop That Runs Until done=TRUE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))           // Is There A Message Waiting?
		{

			if (msg.message == WM_QUIT)               // Have We Received A Quit Message?
			{
				done = TRUE;                  // If So done=TRUE
			}
			else                            // If Not, Deal With Window Messages
			{

				TranslateMessage(&msg);             // Translate The Message
				DispatchMessage(&msg);              // Dispatch The Message
			}
		}
		else                                // If There Are No Messages
		{

			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active)                     // Program Active?
			{
				if (keys[VK_ESCAPE])                // Was ESC Pressed?
				{
					done = TRUE;              // ESC Signalled A Quit
				}
				else                        // Not Time To Quit, Update Screen
				{
					DrawGLScene();              // Draw The Scene
					SwapBuffers(hDC);           // Swap Buffers (Double Buffering)
				}
			}

			if (keys[VK_F1])                    // Is F1 Being Pressed?
			{
				keys[VK_F1] = FALSE;              // If So Make Key FALSE
				KillGLWindow();                 // Kill Our Current Window
				fullscreen = !fullscreen;             // Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("NeHe's OpenGL Framework", windowWidth, windowHeight, 16, fullscreen))
				{
					return 0;               // Quit If Window Was Not Created
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();                             // Kill The Window
	return (msg.wParam);                            // Exit The Program
}