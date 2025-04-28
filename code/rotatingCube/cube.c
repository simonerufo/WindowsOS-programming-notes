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

static BOOL Running = TRUE;
static HGLRC OpenGLRC;

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


void DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight)
{
	//Platform independet OpenGL functions:
	glViewport(0,0, WindowWidth, WindowHeight); // set the viewport
	glClearColor(0.5f, 1.0f, 0.3f, 0.0f); // specify clear values for the color buffers
	glClear(GL_COLOR_BUFFER_BIT); //  clear buffers to preset values
	// The SwapBuffers function exchanges the front and back buffers if the current pixel format for the window
	// referenced by the specified device context includes a back buffer.
	SwapBuffers(DeviceContext); 
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
		InitOpenGL(hWnd);
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
		}

		DestroyOpenGL(OpenGLRC);
	}

	return msg.wParam;
}