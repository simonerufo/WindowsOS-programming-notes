/*
	Notes:
	
		- OpenGL & Win32 api: https://www.youtube.com/watch?v=5Klc9RZPG7M
			- Specific platform operations for windows starts with "wgl"
			- OpenGL RC (render context), the equivalent of windows Device Context 
		- OpenGL 1.0 docs: https://registry.khronos.org/OpenGL/specs/gl/glspec10.pdf

*/

#include <windows.h>
#include <windowsx.h>
#include <gl/gl.h>
#include <math.h>

static BOOL Running = TRUE;
static HGLRC OpenGLRC;
static float Angle = 0.0f; // cube rotation angle
static DWORD lastTime = 0; // last frame timestamp

HGLRC InitOpenGL(HWND hWnd)
{

	HDC hWndDC = GetDC(hWnd);

	// Set up a pixel format descriptor with desired properties
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat); // Size of the structure
    DesiredPixelFormat.nVersion = 1; // Version (always 1)
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER; 
    // Flags: Support OpenGL, draw to window, use double buffering
    DesiredPixelFormat.cColorBits = 32; // 32 bits for color (true color)
    DesiredPixelFormat.cAlphaBits = 8;  // 8 bits for alpha (transparency)
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE; // Main layer (standard)

    // Ask Windows for a pixel format that best matches our requested one
    int SuggestedPixelFormatIndex = ChoosePixelFormat(hWndDC, &DesiredPixelFormat);

    // Retrieve the details of the suggested pixel format
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(hWndDC, SuggestedPixelFormatIndex, 
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);

    // Set the selected pixel format for the device context
    SetPixelFormat(hWndDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    // Create an OpenGL rendering context using the device context
    HGLRC OpenGLRC = wglCreateContext(hWndDC);

    glShadeModel(GL_SMOOTH);

    // Make the OpenGL context current (active) for the device context
    // OpenGL is now ready to use

    if(! wglMakeCurrent(hWndDC, OpenGLRC))
    {
        // ERROR: Failed to activate the OpenGL context
        wglDeleteContext(OpenGLRC);
        OpenGLRC = NULL;
    }


	ReleaseDC(hWnd,hWndDC);

	return OpenGLRC;
} 

void DestroyOpenGL(HGLRC OpenGLRC)
{
    if (OpenGLRC)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(OpenGLRC);
    }
}

void SetPerspective(float fovY, float aspect, float zNear, float zFar)
{
    float fH = tanf(fovY / 360.0f * 3.14159f) * zNear;
    float fW = fH * aspect;
    
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

void DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight)
{
	// framerate handling
	DWORD currentTime = GetTickCount(); 
	float deltaTime = (currentTime - lastTime) / 1000.0f;
	lastTime = currentTime;

	//Platform independet OpenGL functions:
	glViewport(0,0, WindowWidth, WindowHeight); // set the viewport
	
	// camera setup
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	SetPerspective(45.0f, (float)WindowWidth / (float)WindowHeight, 0.1f, 100.0f);

	// transformation setup
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -5.0f); // move cube back
	glRotatef(Angle, 1.0f, 1.0f, 0.0f); // rotate cube


	glClearColor(0.129837f, 0.283764f, 0.54235f, 0.0f); // specify clear values for the color buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //  clear buffers to preset values + Depth buffers (Z-Buffers is a property that  the device uses to store depth infos)
	
	glEnable(GL_DEPTH_TEST); // Enable depth test for 3D

	// draw a cube
	glBegin(GL_QUADS);
	
	// Front face (z+)
	glColor3f(1, 0, 0); // Red
	glVertex3f(-1, -1, 1);
	
	glColor3f(1, 1, 0); // Yellow
	glVertex3f(1, -1, 1);
	
	glColor3f(0, 1, 0); // Green
	glVertex3f(1, 1, 1);
	
	glColor3f(0, 1, 1); // Cyan
	glVertex3f(-1, 1, 1);
	
	// Back face (z-)
	glColor3f(1, 0, 1); // Magenta
	glVertex3f(-1, -1, -1);
	
	glColor3f(0, 0, 1); // Blue
	glVertex3f(-1, 1, -1);
	
	glColor3f(0, 1, 1); // Cyan
	glVertex3f(1, 1, -1);
	
	glColor3f(1, 1, 0); // Yellow
	glVertex3f(1, -1, -1);
	
	// Left face (x-)
	glColor3f(1, 0, 1); // Magenta
	glVertex3f(-1, -1, -1);
	
	glColor3f(1, 0, 0); // Red
	glVertex3f(-1, -1, 1);
	
	glColor3f(0, 1, 0); // Green
	glVertex3f(-1, 1, 1);
	
	glColor3f(0, 0, 1); // Blue
	glVertex3f(-1, 1, -1);
	
	// Right face (x+)
	glColor3f(1, 1, 0); // Yellow
	glVertex3f(1, -1, -1);
	
	glColor3f(0, 1, 1); // Cyan
	glVertex3f(1, 1, -1);
	
	glColor3f(0, 1, 0); // Green
	glVertex3f(1, 1, 1);
	
	glColor3f(1, 0, 0); // Red
	glVertex3f(1, -1, 1);
	
	// Top face (y+)
	glColor3f(0, 1, 1); // Cyan
	glVertex3f(-1, 1, -1);
	
	glColor3f(0, 1, 0); // Green
	glVertex3f(-1, 1, 1);
	
	glColor3f(1, 0, 0); // Red
	glVertex3f(1, 1, 1);
	
	glColor3f(0, 0, 1); // Blue
	glVertex3f(1, 1, -1);
	
	// Bottom face (y-)
	glColor3f(1, 0, 1); // Magenta
	glVertex3f(-1, -1, -1);
	
	glColor3f(1, 1, 0); // Yellow
	glVertex3f(1, -1, -1);
	
	glColor3f(1, 0, 0); // Red
	glVertex3f(1, -1, 1);
	
	glColor3f(0, 1, 1); // Cyan
	glVertex3f(-1, -1, 1);
	
	glEnd();
	
	
	// The SwapBuffers function exchanges the front and back buffers if the current pixel format for the window
	// referenced by the specified device context includes a back buffer.
	SwapBuffers(DeviceContext); 

	Angle += 30.0f * deltaTime; // 45 degrees per second
	if(Angle >= 360.0f) Angle -= 360.0f;

}

void OnDestroy(HWND hWnd)
{
	Running = FALSE;
	PostQuitMessage(0);
}


void OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	RECT rect;
	GetClientRect(hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	DisplayBufferInWindow(hdc, width, height);

	EndPaint(hWnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) 
	{
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
	}

	return DefWindowProc(hWnd, iMsg, wParam, lParam);	
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	
	MSG msg;
	HWND hWnd;
	WNDCLASSEX wc;

	wc.cbSize = sizeof(wc);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("Cube");
	wc.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(
		NULL,
		TEXT("Cube"),
		TEXT("OpenGL Cube"),
		WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	OpenGLRC = InitOpenGL(hWnd);
	
	if(OpenGLRC)
	{
		
        lastTime = GetTickCount();

		Running = TRUE;
		while(Running)
		{	

			while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				if(msg.message == WM_QUIT)
				{
					
					Running = FALSE;
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			InvalidateRect(hWnd, NULL, FALSE);
			Sleep(10);
		}

		DestroyOpenGL(OpenGLRC);
	}

	return msg.wParam;
}