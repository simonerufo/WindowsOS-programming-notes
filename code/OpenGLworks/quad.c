
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdio.h>
#include "glextloader.c"

static GLuint cubeVBO;
static GLuint cubeEBO;

static BOOL Running = FALSE;
static HGLRC OpenGLRC;
static DWORD lastTime = 0;
static float  Angle = 0.0f;

static unsigned int shaderProgram;
static unsigned int vertexShader;
static unsigned int fragmentShader;
static unsigned int VAO;
static unsigned int VBO;
static unsigned int EBO;

// Utility: returns attribute location 0
#define ATTRIB_POSITION 0


const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "}\0";



const char *fragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0f);\n"
    "}\n\0";

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
    RECT rect;
    GetClientRect(hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
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



void CnLShaders()
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

//void InitCubeBuffers()
//{
//    static const float vertices[8][3] = {
//        { -1.0f, -1.0f, -1.0f }, // 0
//        {  1.0f, -1.0f, -1.0f }, // 1
//        {  1.0f,  1.0f, -1.0f }, // 2
//        { -1.0f,  1.0f, -1.0f }, // 3
//        { -1.0f, -1.0f,  1.0f }, // 4
//        {  1.0f, -1.0f,  1.0f }, // 5
//        {  1.0f,  1.0f,  1.0f }, // 6
//        { -1.0f,  1.0f,  1.0f }  // 7
//    };
//
//    static const GLuint indices[36] = {
//        0,1,2,  2,3,0,    // back face
//        4,5,6,  6,7,4,    // front face
//        0,4,7,  7,3,0,    // left face
//        1,5,6,  6,2,1,    // right face
//        3,2,6,  6,7,3,    // top face
//        0,1,5,  5,4,0     // bottom face
//    };
//
//    // VBO gen
//    glGenBuffers(1, &cubeVBO);
//    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//    CheckGLErrors("VBO");
//    // EBO gen
//    glGenBuffers(1, &cubeEBO);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//    CheckGLErrors("EBO");
//
//    // 5) Unbind VBO
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//}

void initTriangleBuffers()
{
    // float vertices[] = 
    // {
    //      0.5f,  0.5f, 0.0f,  // top right
    //      0.5f, -0.5f, 0.0f,  // bottom right
    //     -0.5f, -0.5f, 0.0f,  // bottom left
    //     -0.5f,  0.5f, 0.0f   // top left 
    // };

    float vertices[] = {
        // positions         // colors
         0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // top right    0 
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right 1
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left  2
        -0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f   // top left     3
    };
    unsigned int indices[] = 
    {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };  

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attributes
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),(void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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



void Display(HDC DeviceContext, HWND hWnd)
{
    DWORD currentTime = GetTickCount(); 
    float deltaTime = (currentTime - lastTime) * 0.001f;
    lastTime = currentTime;

    SetupViewport(hWnd);

    glClearColor(0.238974f, 0.2360f, 0.23874f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // use our shader program when we want to render an object
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    // draw the object 
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SwapBuffers(DeviceContext);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) 
	{

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			// DisplayBufferInWindow(hdc, width, height);
		    Display(hdc, hWnd);
			
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
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	
	DebugConsole();

	OpenGLRC = InitOpenGL(hWnd);


    CnLShaders();
    initTriangleBuffers();

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
