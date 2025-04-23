/*
	Drawing stuff to the screen
	Notes:
		- DIB ("Device Independent Bitmap" the modern way which windows handles bitmaps) 
				ref: https://github.com/northern/Win32Bitmaps

		[Steps from tutorial: https://www.youtube.com/watch?v=hNKU8Jiza2g ]
		- VirtualAlloc
		- StretchDIBits
		- Storing pixels in memory
		- Gradient renderer
		- PeekMessage
		- Animating window on the screen


*/

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> 

static char g_szAppName[] = "Gradient";
static char g_szAppTitle[] = "random gradient";

#define	DIB_WIDTH   640
#define	DIB_HEIGHT  480

BYTE* g_pBits = NULL; //bitmap surface
LPBITMAPINFO g_lpBmi = NULL; //



// debug trace
static void __cdecl TRACE(const char *szString, ...)
{
    char szDebugString[1024];
    va_list varList;

    va_start(varList, szString);
    vsnprintf(szDebugString, sizeof(szDebugString), szString, varList);
    va_end(varList);

    OutputDebugStringA(szDebugString);
}

/* 
	cx,cy -> bitmap dimension
	iBpp  -> bit per pixel (8,16,24,32)
	pBits -> [out] it will contain pointer to the buffer of pixels  
*/
LPBITMAPINFO CreateDIB(int cx, int cy, int iBpp, BYTE** pBits)
{
	LPBITMAPINFO lpBmi;
	int iBmiSize; // BITMAPINFO + mask
	int iSurfaceSize; // pixel buffer dimension

	// Calculate the size of the bitmap info header.
	switch(iBpp) {
	case 8:		// 8 bpp
		iBmiSize = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256;
		iSurfaceSize = cx * cy * sizeof(BYTE);
		break;
	
	case 15:	// 15/16 bpp
	case 16:
		iBmiSize = sizeof(BITMAPINFO) + sizeof(DWORD) * 4;
		iSurfaceSize = cx * cy * sizeof(WORD);
		break;

	case 24:	// 24 bpp
		iBmiSize = sizeof(BITMAPINFO);
		iSurfaceSize = cx * cy * (sizeof(BYTE) * 3);
		break;

	case 32:	// 32 bpp
		iBmiSize = sizeof(BITMAPINFO) + sizeof(DWORD) * 4;
		iSurfaceSize = cx * cy * sizeof(DWORD);
		break;
	}

	// Allocate memory for the bitmap info header.
	if((lpBmi = (LPBITMAPINFO)malloc(iBmiSize)) == NULL) {
		TRACE("Error allocating BitmapInfo!\n");
		return NULL;
	}

	ZeroMemory(lpBmi, iBmiSize);

	// Allocate memory for the DIB surface.
	if((*pBits = (BYTE*)malloc(iSurfaceSize)) == NULL) {
		TRACE("Error allocating memory for bitmap bits\n");
		return NULL;
	}

	ZeroMemory(*pBits, iSurfaceSize);

	// Initialize bitmap info header
	lpBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpBmi->bmiHeader.biWidth = cx;
	lpBmi->bmiHeader.biHeight = -(signed)cy;		// <-- NEGATIVE MEANS TOP DOWN!!!
	lpBmi->bmiHeader.biPlanes = 1;
	lpBmi->bmiHeader.biSizeImage = 0;
	lpBmi->bmiHeader.biXPelsPerMeter = 0;
	lpBmi->bmiHeader.biYPelsPerMeter = 0;
	lpBmi->bmiHeader.biClrUsed = 0;
	lpBmi->bmiHeader.biClrImportant = 0;
	lpBmi->bmiHeader.biCompression = BI_RGB;

	// After initializing the bitmap info header we need to store some
	// more information depending on the bpp of the bitmap.
	switch(iBpp) {
	case 8:
		{
			// For the 8bpp DIB we will create a simple grayscale palette.
			for(int i = 0; i < 256; i++) {
				lpBmi->bmiColors[i].rgbRed      = (BYTE)i;
				lpBmi->bmiColors[i].rgbGreen    = (BYTE)i;
				lpBmi->bmiColors[i].rgbBlue     = (BYTE)i;
				lpBmi->bmiColors[i].rgbReserved = (BYTE)0;
			}

			// Set the bpp for this DIB to 8bpp.
			lpBmi->bmiHeader.biBitCount = 8;
		}
		break;
	
	case 15:
		{
			// This is where we will tell the DIB what bits represent what
			// data. This may look confusing at first but the representation
			// of the RGB data can be different on different devices. For
			// example you can have for Hicolor a 565 format. Meaning 5 bits
			// for red, 6 bits for green and 5 bits for blue or better stated
			// like RGB. But, the pixel data can also be the other way around,
			// for example BGR meaning, 5 bits for blue, 6 bits for green and
			// 5 bits for red. This piece of information will tell the bitmap
			// info header how the pixel data will be stored. In this case in
			// RGB format in 555 because this is a 15bpp DIB so the highest
			// bit (bit 15) will not be used.
			DWORD *pBmi = (DWORD*)lpBmi->bmiColors;

			pBmi[0] = 0x00007C00;	// Red mask
			pBmi[1] = 0x000003E0;	// Green mask
			pBmi[2] = 0x0000001F;	// Blue mask
			pBmi[3] = 0x00000000;	// Not used

			// 15bpp DIB also use 16 bits to store a pixel.
			lpBmi->bmiHeader.biBitCount = 16;
			lpBmi->bmiHeader.biCompression |= BI_BITFIELDS;
		}
		break;

	case 16:
		{
			// Take a look at the remarks written by 15bpp. For this format
			// it's the same thing, except in this case the mask's will be
			// different because our format will be 565 (RGB).
			DWORD *pBmi = (DWORD*)lpBmi->bmiColors;

			pBmi[0] = 0x0000F800;	// Red mask
			pBmi[1] = 0x000007E0;	// Green mask
			pBmi[2] = 0x0000001F;	// Blue mask
			pBmi[3] = 0x00000000;	// Not used

			lpBmi->bmiHeader.biBitCount = 16;
			lpBmi->bmiHeader.biCompression |= BI_BITFIELDS;
		}
		break;

	case 24:
		{
			// This is a 1:1 situation. There is no need to set any extra
			// information.
			lpBmi->bmiHeader.biBitCount = 24;
		}
		break;

	case 32:
		{
			// This may speak for it's self. In this case where using 32bpp.
			// The format will be ARGB. the Alpha (A) portion of the format
			// will not be used. The other mask's tell us where the bytes
			// for the R, G and B data will be stored in the DWORD.
			//
			DWORD *pBmi = (DWORD*)lpBmi->bmiColors;

			pBmi[0] = 0x00FF0000;	// Red mask
			pBmi[1] = 0x0000FF00;	// Green mask
			pBmi[2] = 0x000000FF;	// Blue mask
			pBmi[3] = 0x00000000;	// Not used (Alpha?)

			lpBmi->bmiHeader.biBitCount = 32;
			lpBmi->bmiHeader.biCompression |= BI_BITFIELDS;
		}
		break;
	}

	return lpBmi;
}

BOOL OnCreate(HWND hWnd, CREATESTRUCT FAR* lpCreateStruct)
{
	// Create a new 32bpp DIB
	if((g_lpBmi = CreateDIB(DIB_WIDTH, DIB_HEIGHT, 32, &g_pBits)) == NULL) {
		return FALSE;
	}

	// Write a pixel to the DIB surface
	//DWORD* pixel = (DWORD*)g_pBits + (DIB_WIDTH / 2) + ((DIB_HEIGHT / 2) * DIB_WIDTH);
	//*pixel = 0x00FFFFFF; // bianco

	// Write a gradient to the DIB surface
	for (int y = 0; y < DIB_HEIGHT; y++) {
    	for (int x = 0; x < DIB_WIDTH; x++) {
        	DWORD* pixel = (DWORD*)g_pBits + x + y * DIB_WIDTH;
        	BYTE r = (BYTE)(x % 256);					// red depends on x value
        	BYTE g = (BYTE)(y % 256);   				// green depends on y value
        	BYTE b = (BYTE) (x + y  % 256);				// blue depends on x and y values
        	// pixel compisition 0xRR0000 + 0x00GG00 + 0x0000BB
        	*pixel = (r << 16) | (g << 8) | b;
    	}
	}
	return TRUE;
}

void OnDestroy(HWND hWnd)
{
	if(g_pBits) {
		free(g_pBits);
	}

	if(g_lpBmi) {
		free(g_lpBmi);
	}
	
	PostQuitMessage(0);
}

void OnPaint(HWND hWnd)
{
	static PAINTSTRUCT ps;
	static HDC hDC;

	hDC = BeginPaint(hWnd, &ps);

	RECT rc;
	GetClientRect(hWnd, &rc);
	StretchDIBits(hDC, 
				  0, 0, rc.right - rc.left, rc.bottom - rc.top,
				  0, 0, DIB_WIDTH, DIB_HEIGHT, 
				  (BYTE*)g_pBits, 
				  g_lpBmi,
				  DIB_RGB_COLORS, SRCCOPY);

	//SetDIBitsToDevice(hDC, 0, 0, DIB_WIDTH, DIB_HEIGHT, 0, 0, 0, DIB_HEIGHT, (BYTE*)g_pBits, g_lpBmi, DIB_RGB_COLORS);

	EndPaint(hWnd, &ps);
}

BOOL OnEraseBkgnd(HWND hWnd, HDC hdc)
{
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) {
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hWnd, WM_ERASEBKGND, OnEraseBkgnd);
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
	wc.lpszClassName = g_szAppName;
	wc.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(
		NULL,
		g_szAppName,
		g_szAppTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}