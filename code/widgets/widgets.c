#include <windows.h>

// Button ID
#define ID_MYBUTTON 1
#define ID_BUTTON 2
#define RANDOM_TEXT 3

// Window procedure function
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    HWND hwndBtn, hwndBtn2, hwndStatic;

    switch (msg)
    {
    case WM_CREATE:
        // button 1
        hwndBtn =   CreateWindow(
                    "BUTTON",              // Predefined class
                    "Click Me",            // Button text
                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
                    50, 50, 100, 30,       // x, y, width, height
                    hwnd,                 // Parent window
                    (HMENU)ID_MYBUTTON,   // Button ID
                    (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                    NULL);                // No extra data
        // button 2
       hwndBtn2 =   CreateWindow(
                    "BUTTON",              // Predefined class
                    "Don't Click Me",            // Button text
                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
                    50, 100, 100, 30,       // x, y, width, height
                    hwnd,                 // Parent window
                    (HMENU)ID_BUTTON,   // Button ID
                    (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                    NULL); 
        // Static
        hwndStatic =    CreateWindow(
                        "STATIC",
                        NULL,
                        WS_VISIBLE | WS_CHILD | SS_CENTER,
                        10,10,100,20,
                        hwnd,
                        (HMENU)RANDOM_TEXT,
                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                        NULL);  
        
        if (!hwndBtn || !hwndBtn2)

            MessageBox(NULL, "Button failed", "Error", MB_OK | MB_ICONERROR);

        if (!hwndStatic)

            MessageBox(NULL, "Static failed", "Error", MB_OK | MB_ICONERROR);

        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_MYBUTTON)
        {
            MessageBox(hwnd, "You clicked the button!", "Info", MB_OK | MB_ICONINFORMATION);
        }
        else if (LOWORD(wParam) == ID_BUTTON)
        {
            MessageBox(hwnd, "Button clicked", "Info", MB_OK | MB_ICONQUESTION);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    static char className[] = TEXT("MyWindowClass");
    WNDCLASS wc = { 0 } ;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = className;

    if (!RegisterClass(&wc)){
        MessageBox(NULL,"Failed to register  the windows class", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindowEx(
        0,
        className,
        "Window with Buttons",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    AllocConsole();
    printf("%s\n", "Hello");
    HANDLE stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(stdout, "Ciao\r\n", 6, NULL, NULL);
    OutputDebugStringA("ciao"); // debugger 
#define BREAKPOINT __debugbreak()
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
