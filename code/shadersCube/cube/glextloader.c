// https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions
// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-wglgetprocaddress
// https://github.com/tsoding/opengl-template/

static PFNGLCREATESHADERPROC glCreateShader = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
static PFNGLATTACHSHADERPROC glAttachShader = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
static PFNGLDELETESHADERPROC glDeleteShader = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram = NULL;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
static PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = NULL;
static PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
static PFNGLUNIFORM2FPROC glUniform2f = NULL;
static PFNGLGENBUFFERSPROC glGenBuffers = NULL;
static PFNGLBINDBUFFERPROC glBindBuffer = NULL;
static PFNGLBUFFERDATAPROC glBufferData = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
static PFNGLUNIFORM1FPROC glUniform1f = NULL;
static PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;
static PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = NULL;
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;
static PFNGLUNIFORM1IPROC glUniform1i = NULL;
static PFNGLDRAWBUFFERSPROC glDrawBuffers = NULL;
static PFNGLUNIFORM4FPROC glUniform4f = NULL;

static void (*glDisableVertexAttribArray)(GLuint) = NULL;
static void (*glBindAttribLocation)(GLuint, GLuint,GLchar*) = NULL;

// TODO: there is something fishy with Windows gl.h header
// Let's try to ship our own gl.h just like glext.h
#ifdef _WIN32
static PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
#endif // _WIN32

static void load_gl_extensions(void)
{
    // TODO: check for failtures?
    // Maybe some of the functions are not available
    glCreateShader            = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
    glShaderSource            = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
    glCompileShader           = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
    glGetShaderiv             = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog        = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
    glAttachShader            = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
    glCreateProgram           = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
    glLinkProgram             = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
    glGetProgramiv            = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog       = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
    glDeleteShader            = (PFNGLDELETESHADERPROC) wglGetProcAddress("glDeleteShader");
    glUseProgram              = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
    glGenVertexArrays         = (PFNGLGENVERTEXARRAYSPROC) wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray         = (PFNGLBINDVERTEXARRAYPROC) wglGetProcAddress("glBindVertexArray");
    glDeleteProgram           = (PFNGLDELETEPROGRAMPROC) wglGetProcAddress("glDeleteProgram");
    glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
    glUniform2f               = (PFNGLUNIFORM2FPROC) wglGetProcAddress("glUniform2f");
    glGenBuffers              = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
    glBindBuffer              = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
    glBufferData              = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC) wglGetProcAddress("glVertexAttribPointer");
    glUniform1f               = (PFNGLUNIFORM1FPROC) wglGetProcAddress("glUniform1f");
    glBufferSubData           = (PFNGLBUFFERSUBDATAPROC) wglGetProcAddress("glBufferSubData");
    glGenFramebuffers         = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
    glBindFramebuffer         = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D    = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");
    glCheckFramebufferStatus  = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
    glUniform1i               = (PFNGLUNIFORM1IPROC) wglGetProcAddress("glUniform1i");
    glDrawBuffers             = (PFNGLDRAWBUFFERSPROC) wglGetProcAddress("glDrawBuffers");
    glUniform4f               = (PFNGLUNIFORM4FPROC) wglGetProcAddress("glUniform4f");
#ifdef _WIN32
    glActiveTexture           = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress("glActiveTexture");
#endif // _WIN32

    glDisableVertexAttribArray = (void (*)(GLuint)) wglGetProcAddress("glDisableVertexAttribArray");
    glBindAttribLocation = (void (*)  (GLuint, GLuint,GLchar*)) wglGetProcAddress("glBindAttribLocation");
#if 0
    if (glfwExtensionSupported("GL_ARB_debug_output")) {
        fprintf(stderr, "INFO: ARB_debug_output is supported\n");
        glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) wglGetProcAddress("glDebugMessageCallback");
    } else {
        fprintf(stderr, "WARN: ARB_debug_output is NOT supported\n");
    }

    if (glfwExtensionSupported("GL_EXT_draw_instanced")) {
        fprintf(stderr, "INFO: EXT_draw_instanced is supported\n");
        glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC) wglGetProcAddress("glDrawArraysInstanced");
    } else {
        fprintf(stderr, "WARN: EXT_draw_instanced is NOT supported\n");
    }
#endif
}
