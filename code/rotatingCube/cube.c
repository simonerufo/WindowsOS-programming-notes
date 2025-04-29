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


GLuint LoadTextureFromBMP(const char* filename) 
{
    HBITMAP hBitmap;
    BITMAP bmp;
    GLuint textureID;
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);

    // Load the image using Win32 API
    hBitmap = (HBITMAP)LoadImageA(
        NULL, filename, IMAGE_BITMAP, 0, 0,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION
    );

    if (!hBitmap) {
        MessageBoxA(0, "Failed to load BMP file", "Error", MB_OK);
        DeleteDC(memDC);
        ReleaseDC(NULL, hdc);
        return 0;
    }

    // Get basic bitmap info
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    int imageSize = width * height * 3;

    // Create buffer for pixel data
    BYTE* pixels = (BYTE*)malloc(imageSize);

    if (!pixels) {
        MessageBoxA(0, "Failed to allocate memory", "Error", MB_OK);
        DeleteObject(hBitmap);
        DeleteDC(memDC);
        ReleaseDC(NULL, hdc);
        return 0;
    }

    // Prepare BITMAPINFO
    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = -height; // Negative to avoid flipping
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    // Select bitmap into memDC and extract bits
    SelectObject(memDC, hBitmap);
    GetDIBits(memDC, hBitmap, 0, height, pixels, &bi, DIB_RGB_COLORS);

    // Generate texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    /* Note: GL_NEAREST is generally faster than GL_LINEAR, 
    but it can produce textured images with sharper edges 
    because the transition between texture elements is not as smooth. 
    The default value of GL_TEXTURE_MAG_FILTER is GL_LINEAR.
    */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load bitmap to OpenGL
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_BGR_EXT,
        GL_UNSIGNED_BYTE,
        pixels
    );

    // Cleanup
    free(pixels);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, hdc);

    return textureID;
}


void SetPerspective(float fovY, float aspect, float zNear, float zFar)
{
	/*

	fovY: Field of view (vertical) in degrees (for example: 45°).
	aspect: Aspect ratio of the window = width / height.
	zNear: Distance to the near clipping plane.
	zFar: Distance to the far clipping plane.
	
	*/
    float fH = tanf((fovY * 0.5f) * (3.14159f / 180.0f)) * zNear;

    float fW = fH * aspect;
    
    // set the OpenGL 3D perspective
    // gluPerspective() does the same, without manual calculations (GL\glu.h required)
    glFrustum(-fW, fW, -fH, fH, zNear, zFar); 
}

void DrawRainbowCube()
{
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
}

void DrawTextureCube(const char* filename)
{
	GLuint texture = LoadTextureFromBMP(filename);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture); 
	
	// draw a cube
	glBegin(GL_QUADS);
	
	// Front face (z+)
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1, -1, 1);
	
	glTexCoord2f(1.0f, 0.0f); 
	glVertex3f(1, -1, 1);
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1, 1, 1);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1, 1, 1);
	
	// Back face (z-)
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1, -1, -1);
	
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1, 1, -1);
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1, 1, -1);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1, -1, -1);
	
	// Left face (x-)
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1, -1, -1);
	
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1, -1, 1);
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1, 1, 1);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1, 1, -1);
	
	// Right face (x+)
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1, -1, -1);
	
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1, 1, -1);
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1, 1, 1);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1, -1, 1);
	
	// Top face (y+)
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1, 1, -1);
	
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1, 1, 1);
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1, 1, 1);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1, 1, -1);
	
	// Bottom face (y-)
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1, -1, -1);
	
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1, -1, -1);
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1, -1, 1);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1, -1, 1);
	
	glEnd();
}

void DrawTextureGrass()
{
	GLuint bTexture = LoadTextureFromBMP(".\\dirt.bmp");
	GLuint sTexture = LoadTextureFromBMP(".\\dirtgrass.bmp");
	GLuint tTexture = LoadTextureFromBMP(".\\grass.bmp");

	// Enable texturing
	glEnable(GL_TEXTURE_2D);
		
	
	// Front face (z+)
	glBindTexture(GL_TEXTURE_2D, sTexture);
	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex3f(1, 1, 1);
	
	glTexCoord2f(1, 0); 
	glVertex3f(-1, 1, 1);
	
	glTexCoord2f(1, 1);
	glVertex3f(-1, -1, 1);
	
	glTexCoord2f(0, 1);
	glVertex3f(1, -1, 1);
	
	glEnd();
	 
	// Back face (z-)
	glBindTexture(GL_TEXTURE_2D, sTexture);
	glBegin(GL_QUADS);

	glTexCoord2f(1, 0);
	glVertex3f(-1, 1, -1);
	
	glTexCoord2f(0, 0);
	glVertex3f(1, 1, -1);
	
	glTexCoord2f(0, 1);
	glVertex3f(1, -1, -1);
	
	glTexCoord2f(1, 1);
	glVertex3f(-1, -1, -1);
	
	glEnd();
	
	// Left face (x-)
	glBindTexture(GL_TEXTURE_2D, sTexture);
	glBegin(GL_QUADS);
	
	glTexCoord2f(0, 1);
	glVertex3f(-1, -1, -1);
	
	glTexCoord2f(1, 1);
	glVertex3f(-1, -1, 1); 
	
	glTexCoord2f(1, 0);
	glVertex3f(-1, 1, 1); 
	
	glTexCoord2f(0, 0);
	glVertex3f(-1, 1, -1); 
	
	glEnd();

	// Right face (x+)
	glBindTexture(GL_TEXTURE_2D, sTexture);
	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex3f(1, 1, 1);
	
	glTexCoord2f(1, 0);
	glVertex3f(1, 1, -1);
	
	glTexCoord2f(1, 1);
	glVertex3f(1, -1, -1);
	
	glTexCoord2f(0, 1);
	glVertex3f(1, -1, 1);
	
	glEnd();

	// Top face (y+)
	glBindTexture(GL_TEXTURE_2D, tTexture);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1, 1, -1);
	
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1, 1, 1);
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1, 1, 1);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1, 1, -1);
	
	glEnd();

	// Bottom face (y-)
	glBindTexture(GL_TEXTURE_2D, bTexture);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1, -1, -1);
	
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1, -1, -1);
	
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1, -1, 1);
	
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1, -1, 1);
	
	glEnd();
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


	// DrawRainbowCube();
	// DrawTextureCube(".\\dirt.bmp");
	DrawTextureGrass();

	// The SwapBuffers function exchanges the front and back buffers if the current pixel format for the window
	// referenced by the specified device context includes a back buffer.
	SwapBuffers(DeviceContext); 

	Angle += 90.0f * deltaTime; // 30 degrees per second
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