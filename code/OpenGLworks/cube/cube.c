#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdlib.h>
#include <stdio.h>
#include "glextloader.c"
#include "matrix.c"

BOOL Running = FALSE;
BOOL firstMouse = TRUE;
HGLRC OpenGLRC = NULL;
DWORD lastTime = 0;
float deltaTime = 0;
float  Angle = 0.0f;
float yaw = -90.0f;
float pitch = 0;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
unsigned int VBO = 0;
unsigned int VAO = 0;
unsigned int shaderProgram = 0;
unsigned int vertexShader = 0;
unsigned int fragmentShader = 0;
GLuint texture = 0;

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
float cubePositions[] = 
    {
        // x    // y    // z
         0.0f,  0.0f,   0.0f,
         2.0f,  5.0f, -15.0f,
        -1.5f, -2.2f,  -2.5f,
        -3.8f, -2.0f, -12.3f,
         2.4f, -0.4f,  -3.5f,
        -1.7f,  3.0f,  -7.5f,
         1.3f, -2.0f,  -2.5f,
         1.5f,  2.0f,  -2.5f,
         1.5f,  0.2f,  -1.5f,
        -1.3f,  1.0f,  -1.5f
    };
float cameraPosition[] = 
    {
        0.0f, 0.0f, 3.0f
    };
float cameraFront[] = 
    {
        0.0f, 0.0f, -1.0f
    };
float cameraUp[] = 
    {
        0.0f, 1.0f, 0.0f
    };
float direction[] = 
    {
        0.0f, 0.0f, 0.0f
    };

const char* vertexShaderSource = 
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
	"	TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);\n"
	"}\0";

const char* fragmentShaderSource =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec2 TexCoord;\n"
	"uniform sampler2D texture1;\n"
	"void main()\n"
	"{\n"
	"	FragColor = texture(texture1, TexCoord);\n"
	"}\0";


void 
getScreenDim_Win32(HWND hWnd, int *width, int* height) // 32
{
    RECT rect;
    GetClientRect(hWnd, &rect);
    *width = rect.right - rect.left;
    *height = rect.bottom - rect.top;
}

void 
CheckGLErrors(const char *context) // 32
{
    GLenum err;
    while ( (err = glGetError()) != GL_NO_ERROR)
    {
        printf("[OpenGL Error] (%s): 0x%X\n", context, err);
    }
}

GLuint 
LoadTextureFromBMP_Win32(const char* filename) // 32
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

void 
SetupViewport(HWND hWnd)
{
    int width, height;
    getScreenDim_Win32(hWnd, &width, &height);
    glViewport(0, 0, width, height);
    CheckGLErrors("Viewport");
}

void 
CompileAndLinkShaders()
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

void 
BindVertexArrays()
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

void 
LoadAndCreateTextures() // 32
{
    texture = LoadTextureFromBMP_Win32("dirt.bmp");
    if (texture == 0) {
        fprintf(stderr, "Failed to load texture!\n");
        return;
    }
    // glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    CheckGLErrors("Bind texture");

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    CheckGLErrors("Texture wrapping");

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CheckGLErrors("Texture filtering");
}



void 
Display(HDC DeviceContext, HWND hWnd, int width, int height)
{
    // time setup
	DWORD currentTime = GetTickCount(); 
    deltaTime = (currentTime - lastTime) * 0.001f;
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

    mat4_rotate(model, Angle, 1.0f, 1.0f, 0.0f);
    mat4_translate(view, 0.0f, 0.0f, -3.0f);
    mat4_perspective(projection, 3.1415926f/4.0f, (float)width/(float)height, 0.1f, 100.0f);

    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc  = glGetUniformLocation(shaderProgram, "view");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    GLint loc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &projection[0][0]);

    // render box
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    SwapBuffers(DeviceContext);

    Angle += 0.5f * deltaTime;
    if (Angle >= 360.0f) Angle = 0.0f;
}

void 
DisplayMultiple(HDC DeviceContext, HWND hWnd, int width, int height)
{
    DWORD currentTime = GetTickCount(); 
    deltaTime = (currentTime - lastTime) * 0.001f;
    lastTime = currentTime;

    SetupViewport(hWnd);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, texture);
    glUseProgram(shaderProgram);

    mat4 view, projection;
    mat4_identity(view);
    mat4_translate(view, 0.0f, 0.0f, -3.0f);
    mat4_identity(projection);
    mat4_perspective(projection, 3.1415926f/4.0f, (float)width/height, 0.1f, 100.0f);

    unsigned int viewLoc       = glGetUniformLocation(shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(viewLoc,       1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);
    
    unsigned int modelLoc      = glGetUniformLocation(shaderProgram, "model");
    float frameAngle = Angle;

    glBindVertexArray(VAO);
    for (unsigned int i = 0; i < 10; ++i)
    {
        float x = cubePositions[i * 3 + 0];
        float y = cubePositions[i * 3 + 1];
        float z = cubePositions[i * 3 + 2];
        
        
        mat4 model;
        mat4_identity(model);

        float thisAngle = frameAngle + i * 20.0f;
        mat4_rotate_inplace(model, radians(thisAngle), 1.0f, 0.3f, 0.1f);
        mat4_translate_inplace(model, x, y, z);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    Angle += 10.0f * deltaTime;
    if(Angle >= 360.0f) Angle -= 360.0f;
    
    SwapBuffers(DeviceContext);

}

void 
DisplayMultipleCamera(HDC DeviceContext, HWND hWnd, int width, int height)
{
    DWORD currentTime = GetTickCount(); 
    deltaTime = (currentTime - lastTime) * 0.001f;
    lastTime = currentTime;

    SetupViewport(hWnd);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, texture);
    glUseProgram(shaderProgram);

    mat4 view;
    mat4_lookAt(view,
                cameraPosition[0], cameraPosition[1], cameraPosition[2],
                cameraPosition[0] + cameraFront[0], cameraPosition[1] + cameraFront[1], cameraPosition[2] + cameraFront[2],
                cameraUp[0], cameraUp[1], cameraUp[2]); 

    mat4 projection;
    mat4_perspective(projection, 3.1415926f/4.0f, (float)width/height, 0.1f, 100.0f);

    unsigned int viewLoc       = glGetUniformLocation(shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(viewLoc,       1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);
    
    unsigned int modelLoc      = glGetUniformLocation(shaderProgram, "model");
    float frameAngle = Angle;

    glBindVertexArray(VAO);
    for (unsigned int i = 0; i < 10; ++i)
    {
        float x = cubePositions[i * 3 + 0];
        float y = cubePositions[i * 3 + 1];
        float z = cubePositions[i * 3 + 2];
        
        
        mat4 model;
        mat4_identity(model);

        float thisAngle = frameAngle + i * 20.0f;
        mat4_rotate_inplace(model, radians(thisAngle), 1.0f, 0.3f, 0.1f);
        mat4_translate_inplace(model, x, y, z);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    Angle += 10.0f * deltaTime;
    if(Angle >= 360.0f) Angle -= 360.0f;
    
    SwapBuffers(DeviceContext);

}

void 
DebugConsole() // 32
{
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	SetConsoleTitleA("Debug Console");
	
	printf("Debug console initialized.\n");
}

HDC 
SetupPixelFormat(HWND hWnd) // 32
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


HGLRC 
InitOpenGL(HWND hWnd) // 32
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

void 
DestroyOpenGL(HGLRC OpenGLRC) // 32
{
    if (OpenGLRC)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(OpenGLRC);
    }
}

void 
cameraMovement(WPARAM wParam)
{
    float cameraSpeed = 5.0f * deltaTime;
    // WM_KEYDOWN: 0x57 'W' 
    if(wParam == 0x57)
    {
        cameraPosition[0] += cameraSpeed * cameraFront[0];
        cameraPosition[1] += cameraSpeed * cameraFront[1];
        cameraPosition[2] += cameraSpeed * cameraFront[2];
    }
    // WM_KEYDOWN: 0x41 'A'
    if(wParam == 0x41)
    {
        float sideX, sideY, sideZ;
        
        cross(
                cameraFront[0], cameraFront[1], cameraFront[2],
                   cameraUp[0],    cameraUp[1],    cameraUp[2],
                        &sideX,         &sideY,         &sideZ
             );
        
        normalize(&sideX, &sideY, &sideZ);
        
        cameraPosition[0] -= sideX * cameraSpeed;
        cameraPosition[1] -= sideY * cameraSpeed;
        cameraPosition[2] -= sideZ * cameraSpeed;
    }
    // WM_KEYDOWN: 0x44 'D' 
    if(wParam == 0x44)
    {
        float sideX, sideY, sideZ;
        cross(
                cameraFront[0], cameraFront[1], cameraFront[2],
                   cameraUp[0],    cameraUp[1],    cameraUp[2],
                        &sideX,         &sideY,         &sideZ
             );
        
        normalize(&sideX, &sideY, &sideZ);
        
        cameraPosition[0] += sideX * cameraSpeed;
        cameraPosition[1] += sideY * cameraSpeed;
        cameraPosition[2] += sideZ * cameraSpeed;
    }
    // WM_KEYDOWN: 0x53 'S'
     if(wParam == 0x53)
    {
        cameraPosition[0] -= cameraSpeed * cameraFront[0];
        cameraPosition[1] -= cameraSpeed * cameraFront[1];
        cameraPosition[2] -= cameraSpeed * cameraFront[2];
    }
}


void 
mouseMovement(float x, float y)
{
    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = FALSE;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y;
    
    lastX = x;
    lastY = y;

    const float sensitivity = 0.3f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;
    
    if(pitch > 89.0f)
      pitch =  89.0f;
    if(pitch < -89.0f)
      pitch = -89.0f;

    direction[0] = cos(radians(yaw) * cos(radians(pitch)));
    direction[1] = sin(radians(pitch));
    direction[2] = sin(radians(yaw)) * cos(radians(pitch));
    
    normalize(&direction[0], &direction[1], &direction[2]);

    cameraFront[0] = direction[0];
    cameraFront[1] = direction[1];
    cameraFront[2] = direction[2]; 
}

void 
LockCursorToWindow(HWND hwnd) 
{
    RECT rect;
    GetClientRect(hwnd, &rect);           
    POINT ul = { rect.left, rect.top };
    POINT lr = { rect.right, rect.bottom };
    ClientToScreen(hwnd, &ul);            
    ClientToScreen(hwnd, &lr);
    rect.left   = ul.x;
    rect.top    = ul.y;
    rect.right  = lr.x;
    rect.bottom = lr.y;

    ClipCursor(&rect);   
    ShowCursor(FALSE);                
}

LRESULT CALLBACK 
WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) 
	{

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

            int width, height;
            getScreenDim_Win32(hWnd, &width, &height);
		    // Display(hdc, hWnd, width, height);
			// DisplayMultiple(hdc, hWnd, width, height);
            DisplayMultipleCamera(hdc, hWnd, width, height);
            EndPaint(hWnd, &ps);
			break;
		}

        case WM_KEYDOWN:
        {
            if(wParam == VK_ESCAPE)
                PostMessage(hWnd, WM_CLOSE, 0, 0);

            cameraMovement(wParam);
                
            break;
        }

        case WM_MOUSEMOVE: 
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            mouseMovement(x, y);
            break;
        }

        case WM_DESTROY:
        {
            Running = FALSE;
            PostQuitMessage(0);
        }

	}

	return DefWindowProc(hWnd, iMsg, wParam, lParam);	
}

int WINAPI 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
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
		0,
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
    BindVertexArrays();
    LoadAndCreateTextures();
    LockCursorToWindow(hWnd);

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
