/*
	Displays Hello in client area
*/

#include <windows.h>

// window procedure (win prock.)
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     static TCHAR szAppName[] = TEXT ("HelloWin") ;
     HWND         hwnd ;
     MSG          msg ;
     WNDCLASS     wndclass ;

     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ; // loads an icon for use by a program
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ; // loads a cursor for use by a program
     wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ; // obtains a graphic object (in this case it painted the bg)
     wndclass.lpszMenuName  = NULL ;
     wndclass.lpszClassName = szAppName ;

     if (!RegisterClass (&wndclass)) // registers a window class for the program's window
     {
          MessageBox (NULL, TEXT ("This program requires Windows NT!"), // displays a message box
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }
     
     // creates a window based on a window class
     hwnd = CreateWindow (szAppName,                  // window class name
                          TEXT ("The Hello Program"), // window caption
                          WS_OVERLAPPEDWINDOW,        // window style
                          CW_USEDEFAULT,              // initial x position
                          CW_USEDEFAULT,              // initial y position
                          CW_USEDEFAULT,              // initial x size
                          CW_USEDEFAULT,              // initial y size
                          NULL,                       // parent window handle
                          NULL,                       // window menu handle
                          hInstance,                  // program instance handle
                          NULL) ;                     // creation parameters
     
     ShowWindow (hwnd, iCmdShow) ; // shows the window on the screen
     UpdateWindow (hwnd) ; // directs the window to paint itself
     
     while (GetMessage (&msg, NULL, 0, 0)) // obtains a message from the message queue
     {
          TranslateMessage (&msg) ; // translates some keyboard messages
          DispatchMessage (&msg) ; // sends a message to a window procedure 
     }
     return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     HDC         hdc ;
     PAINTSTRUCT ps ;
     RECT        rect ;
     
     switch (message)
     {
     case WM_CREATE:
          PlaySound (TEXT ("hellowin.wav"), NULL, SND_FILENAME | SND_ASYNC) ; // plays a sound file...
          return 0 ;
          
     case WM_PAINT:
          hdc = BeginPaint (hwnd, &ps) ; // initiates the beginning of window painting
          
          GetClientRect (hwnd, &rect) ; // obtains the dimensions of the window's client area
          
          DrawText (hdc, TEXT ("Hello, Windows 98!"), -1, &rect, // displays a text string
                    DT_SINGLELINE | DT_CENTER | DT_VCENTER) ;
          
          EndPaint (hwnd, &ps) ; // ends window painting
          return 0 ;
          
     case WM_DESTROY:
          PostQuitMessage (0) ; // inserts a "quit" message into the message queue
          return 0 ;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ; // performs default processing of messages
}