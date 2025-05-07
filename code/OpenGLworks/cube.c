
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdio.h>
#include "glextloader.c"

static GLuint cubeVBO;
static GLuint cubeEBO;
static GLuint shaderProgram;
static BOOL Running = FALSE;
static HGLRC OpenGLRC;
static DWORD lastTime = 0;
static float  Angle = 0.0f; 

// Utility: returns attribute location 0
#define ATTRIB_POSITION 0


const char* vertexShaderSource = 
"#version 120\n"
"attribute vec3 position;\n"
"void main() {\n"
"    gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);\n"
"}\n";


const char* fragmentShaderSource = 
"#version 120\n"
"void main() {\n"
"    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\n";

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


void CompileAndLinkShaders()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    CheckGLErrors("Compile vertex shader");

    // Vertex error log
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
        printf("[Vertex Shader ERROR]\n%s\n", infoLog);
    } else {
        printf("[Vertex Shader] Successfully compiled.\n");
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Fragment error log
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
        printf("[Fragment Shader ERROR]\n%s\n", infoLog);
    } else {
        printf("[Fragment Shader] Successfully compiled.\n");
    }

    shaderProgram = glCreateProgram();  
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindAttribLocation(shaderProgram, ATTRIB_POSITION, "position");
    glLinkProgram(shaderProgram);
    CheckGLErrors("Link shader program");

    // Linking error log
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        printf("[Shader Program LINK ERROR]\n%s\n", infoLog);
    } else {
        printf("[Shader Program] Successfully linked.\n");
    }

    
    glUseProgram(shaderProgram);
    CheckGLErrors("Shader program");

    // Cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void InitCubeBuffers()
{
    static const float vertices[8][3] = {
        { -1.0f, -1.0f, -1.0f }, // 0
        {  1.0f, -1.0f, -1.0f }, // 1
        {  1.0f,  1.0f, -1.0f }, // 2
        { -1.0f,  1.0f, -1.0f }, // 3
        { -1.0f, -1.0f,  1.0f }, // 4
        {  1.0f, -1.0f,  1.0f }, // 5
        {  1.0f,  1.0f,  1.0f }, // 6
        { -1.0f,  1.0f,  1.0f }  // 7
    };

    static const GLuint indices[36] = {
        0,1,2,  2,3,0,    // back face
        4,5,6,  6,7,4,    // front face
        0,4,7,  7,3,0,    // left face
        1,5,6,  6,2,1,    // right face
        3,2,6,  6,7,3,    // top face
        0,1,5,  5,4,0     // bottom face
    };

    // VBO gen
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    CheckGLErrors("VBO");
    // EBO gen
    glGenBuffers(1, &cubeEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    CheckGLErrors("EBO");

    // 5) Unbind VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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

void DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    // 1) Frame timing
    DWORD currentTime = GetTickCount(); 
    float deltaTime = (currentTime - lastTime) * 0.001f;
    lastTime = currentTime;



    // 2) Viewport & projection
    glViewport(0, 0, WindowWidth, WindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (WindowHeight == 0) WindowHeight = 1;
    SetPerspective(45.0f, (float)WindowWidth / WindowHeight, 0.1f, 100.0f);
    CheckGLErrors("glFrustum");

    // 3) Model transform
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(Angle, 1.0f, 1.0f, 0.0f);

    // 4) Clear
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.129837f, 0.283764f, 0.54235f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CheckGLErrors("glClear");


    // 5) Use shader + bind buffers
    if (shaderProgram == 0) {
        printf("[ERROR] shaderProgram is NULL\n");
        return;
    }

    glUseProgram(shaderProgram);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
 	CheckGLErrors("AttribArray");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);

    // 6) Draw
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    GLenum err = glGetError();
	CheckGLErrors("Draw cube");


    // 7) Cleanup
    glDisableVertexAttribArray(ATTRIB_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);


    // 8) Present and update rotation
    SwapBuffers(DeviceContext);
    CheckGLErrors("SwapBuffers");
    Angle += 90.0f * deltaTime;
    if (Angle >= 360.0f) Angle -= 360.0f;

}



LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) 
	{

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
		
			RECT rect;
			GetClientRect(hWnd, &rect);
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
		
			DisplayBufferInWindow(hdc, width, height);
		
			EndPaint(hWnd, &ps);
			break;
		}

		case WM_DESTROY:
		{

			//Running = FALSE;
			//PostQuitMessage(0);
			break;
		}

		case WM_LBUTTONDOWN: 
		{
			int xPos = GET_X_LPARAM(lParam);
    		int yPos = GET_Y_LPARAM(lParam);
    		RECT rect;

    		GetClientRect(hWnd, &rect);
    		int height = rect.bottom - rect.top;
    		
    		// OnMouseClick(hWnd, xPos, yPos, height);
    		break;
		}
	}

	return DefWindowProc(hWnd, iMsg, wParam, lParam);	
}

void APIENTRY 
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	// __debugbreak();
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
		800,
		600,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	
	DebugConsole();

	OpenGLRC = InitOpenGL(hWnd);

	glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback( MessageCallback, 0 );

	CompileAndLinkShaders();
	InitCubeBuffers();

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
