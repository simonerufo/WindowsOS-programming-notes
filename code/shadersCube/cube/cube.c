#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdlib.h>
#include <stdio.h>
#include "glextloader.c"
#include "matrix.c"

static BOOL Running = FALSE;
static HGLRC OpenGLRC = NULL;
static DWORD lastTime = 0;
static float  Angle = 0.0f;
static unsigned int VBO = NULL;
static unsigned int VAO = NULL;
static unsigned int shaderProgram = NULL;

/*
	notes: initCube bindText compileAndLinkShaders are not used!!
    Workflow:
    - InitOpenGL
    - CompileAndLinkShader
    - InitCube
    - BindVertexArrays
    - LoadAndCreateTextures
    - Display

*/
const char* vertexShader = 
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec2 aTexCoord;\n"
	"out vec2 TexCoord;\n"
	"uniform mat4 model;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
	"	TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
	"}\0";

const char* fragmentShader =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec2 TexCoord;\n"
	"uniform sampler2D texture1;\n"
	"void main()\n"
	"{\n"
	"	FragColor = texture(texture1, TexCoord);\n"
	"}\0";


void getScreenDim_Win32(HWND hWnd, int* width, int* height)
{
    RECT rect;
    GetClientRect(hWnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    printf("w: %d, h: %d\n", width, height);
}
GLuint LoadTextureFromBMP_Win32(const char* filename)
{
    HBITMAP    hBitmap = NULL;
    BITMAP     bmp;
    BYTE       *pixels = NULL;
    GLuint     texID = 0;
    HDC        screenDC = NULL;
    HDC        memDC = NULL;

    // Acquire DCs
    screenDC = GetDC(NULL);
    if (!screenDC) {
        fprintf(stderr, "Error: GetDC failed\n");
        return 0;
    }
    memDC = CreateCompatibleDC(screenDC);
    if (!memDC) {
        fprintf(stderr, "Error: CreateCompatibleDC failed\n");
        ReleaseDC(NULL, screenDC);
        return 0;
    }

    // Load BMP into DIB section (24-bit)
    hBitmap = (HBITMAP)LoadImageA(
        NULL, filename, IMAGE_BITMAP,
        0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION
    );
    if (!hBitmap) {
        fprintf(stderr, "Error: could not load BMP \"%s\"\n", filename);
        DeleteDC(memDC);
        ReleaseDC(NULL, screenDC);
        return 0;
    }

    // Query bitmap info
    GetObject(hBitmap, sizeof(bmp), &bmp);
    if (bmp.bmBitsPixel != 24) {
        fprintf(stderr, "Error: only 24-bit BMP supported (found %u bpp)\n",
                bmp.bmBitsPixel);
        DeleteObject(hBitmap);
        DeleteDC(memDC);
        ReleaseDC(NULL, screenDC);
        return 0;
    }

    // Allocate pixel buffer
    pixels = (BYTE*)malloc(bmp.bmWidth * bmp.bmHeight * 3);
    if (!pixels) {
        fprintf(stderr, "Error: malloc(%u) failed\n",
                bmp.bmWidth * bmp.bmHeight * 3u);
        DeleteObject(hBitmap);
        DeleteDC(memDC);
        ReleaseDC(NULL, screenDC);
        return 0;
    }

    // Prepare BITMAPINFO for top-down BGR read
    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize        = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth       = bmp.bmWidth;
    bi.bmiHeader.biHeight      = -bmp.bmHeight;  // negative for top-down
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    // Extract bits
    SelectObject(memDC, hBitmap);
    if (!GetDIBits(memDC, hBitmap, 0, bmp.bmHeight, pixels, &bi, DIB_RGB_COLORS)) {
        fprintf(stderr, "Error: GetDIBits failed\n");
        free(pixels);
        DeleteObject(hBitmap);
        DeleteDC(memDC);
        ReleaseDC(NULL, screenDC);
        return 0;
    }

    // Create and upload OpenGL texture
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB8,
        bmp.bmWidth,
        bmp.bmHeight,
        0,
        GL_BGR,
        GL_UNSIGNED_BYTE,
        pixels
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    // Cleanup GDI & memory
    free(pixels);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, screenDC);

    return texID;
}

void CompileAndLinkShaders()
{
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    CheckGLErrors("Compile vertex shader");


    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    CheckGLErrors("Compile fragment shader");


    shaderProgram = glCreateProgram(); 
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    CheckGLErrors("shader program link");

    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);  
}

void BindVertexArrays()
{
	glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    CheckGLErrors("VBO Bind");

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    CheckGLErrors("Vertex Attribute position");
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    CheckGLErrors("Vertex Attribute texture");
}

void LoadAndCreateTextures()
{
	unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    CheckGLErrors("Bind texture");

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    CheckGLErrors("Texture wrapping");

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CheckGLErrors("Texture filtering");

    // load image, create texture and generate mipmaps
    GLuint data = LoadTextureFromBMP_Win32("dirt.bmp");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    CheckGLErrors("Image load and texture creation");
}

void initCubeVertex()
{
	float vertices[] = 
	{
        	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	
        	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	
        	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	
        	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	
        	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	
        	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
}

void Display(HDC DeviceContext, HWND hWnd, width, height)
{
    // time setup
	DWORD currentTime = GetTickCount(); 
    float deltaTime = (currentTime - lastTime) * 0.001f;
    lastTime = currentTime;

    SetupViewport(hWnd);

    // render 
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // activate shader
    glUseProgram(shaderProgram);

    // transformations
    mat4 model, view, projection;
    mat4_identity(model);
    mat4_identity(view);
    mat4_identity(projection);

    mat4_rotate(model, deltaTime, 0.5f, 1.0f, 0.0f);
    mat4_translate(view, 0.0f, 0.0f, -3.0f);
    mat4_perspective(projection, 45.0f, width/height, 0.1f, 100.0f);

    unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
    unsigned int viewLoc  = glGetUniformLocation(ourShader.ID, "view");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    GLint loc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &projection[0][0]);

    // render box
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    SwapBuffers(DeviceContext);

}

static void DebugConsole()
{
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	SetConsoleTitleA("Shader Debug Console");
	
	printf("Debug console initialized.\n");
}

void CheckGLErrors(const char* context) 
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        printf("[OpenGL Error] (%s): 0x%X\n", context, err);
    }
}

HDC SetupPixelFormat(HWND hWnd)
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
    DesiredPixelFormat.cDepthBits = 24;  // depth buffer


    // Ask Windows for a pixel format that best matches our requested one
    int SuggestedPixelFormatIndex = ChoosePixelFormat(hWndDC, &DesiredPixelFormat);

    // Retrieve the details of the suggested pixel format
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(hWndDC, SuggestedPixelFormatIndex, 
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);

    // Set the selected pixel format for the device context
    SetPixelFormat(hWndDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    return hWndDC;
}

void SetupViewport(HWND hWnd)
{
    int width, height;
    getScreenDim_Win32(HWND hWnd, &width, &height);
    glViewport(0, 0, width, height);
    CheckGLErrors("Viewport");
}

HGLRC InitOpenGL(HWND hWnd)
{
	HDC hWndDC = SetupPixelFormat(hWnd);
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
    
    load_gl_extensions();

	ReleaseDC(hWnd,hWndDC);

    glEnable(GL_DEPTH_TEST);

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

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) 
	{

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

            int width, height;
            getScreenDim_Win32(hWnd, &width, &height);
		    Display(hdc, hWnd, width, height);
			
            EndPaint(hWnd, &ps);
			break;
		}

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
	
	DebugConsole();

	OpenGLRC = InitOpenGL(hWnd);
    CompileAndLinkShaders();
    initCubeVertex();
    BindVertexArrays();
    LoadAndCreateTextures();


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
