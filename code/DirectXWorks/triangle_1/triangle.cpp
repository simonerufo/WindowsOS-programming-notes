#define UNICODE
#define _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <d3dcommon.h> 
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
ID3D11Device *dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer;    // the pointer to our back buffer
ID3D11InputLayout *pLayout;            // the pointer to the input layout
ID3D11VertexShader *pVS;               // the pointer to the vertex shader
ID3D11PixelShader *pPS;                // the pointer to the pixel shader
ID3D11Buffer *pVBuffer;                // the pointer to the vertex buffer
ID3D11Buffer *indexBuffer;
ID3D11Buffer* matrixBuffer;
ID3D11DepthStencilView* depthStencilView;
ID3D11DepthStencilState* depthStencilState;
ID3D11RasterizerState* pRS;

float  gRotationAngle = 0.0f;       
float  gRotationSpeed = XM_PIDIV4;
LARGE_INTEGER gTimerFreq;           // frequenza del timer
LARGE_INTEGER gPrevTime;            // ultimo timestamp

struct VERTEX{
			FLOAT X, Y, Z; 
			FLOAT R, G, B, A;
		};

struct MatrixBufferType
{
    XMMATRIX model;
    XMMATRIX view;
    XMMATRIX projection;
};

// prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void RenderFrame(void);     // renders a single frame
void CleanD3D(void);        // closes Direct3D and releases memory
void InitGraphics(void);    // creates the shape to render
void InitPipeline(void);    // loads and prepares the shaders
void InitMatrix(void);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    RECT wr = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL,
                          L"WindowClass",
                          L"Direct3D Program",
                          WS_OVERLAPPEDWINDOW,
                          300,
                          300,
                          wr.right - wr.left,
                          wr.bottom - wr.top,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

    InitD3D(hWnd);

    MSG msg;

    while(TRUE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT)
                break;
        }

        RenderFrame();
    }

    // clean up DirectX and COM
    CleanD3D();

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}

void InitD3D(HWND hWnd)
{

    QueryPerformanceFrequency(&gTimerFreq);
    QueryPerformanceCounter(&gPrevTime);

    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    // clear out the struct for use
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    // fill the swap chain description struct
    scd.BufferCount = 1;                                   // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // use 32-bit color
    scd.BufferDesc.Width = SCREEN_WIDTH;                   // set the back buffer width
    scd.BufferDesc.Height = SCREEN_HEIGHT;                 // set the back buffer height
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;     // how swap chain is to be used
    scd.OutputWindow = hWnd;                               // the window to be used
    scd.SampleDesc.Count = 4;                              // how many multisamples
    scd.Windowed = TRUE;                                   // windowed/full-screen mode
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    // allow full-screen switching

    // create a device, device context and swap chain using the information in the scd struct
    D3D11CreateDeviceAndSwapChain(NULL,
                                  D3D_DRIVER_TYPE_HARDWARE,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  D3D11_SDK_VERSION,
                                  &scd,
                                  &swapchain,
                                  &dev,
                                  NULL,
                                  &devcon);


    // get the address of the back buffer
    ID3D11Texture2D *pBackBuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // use the back buffer address to create the render target
    dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
    pBackBuffer->Release();

    // set the render target as the back buffer
    //devcon->OMSetRenderTargets(1, &backbuffer, depthStencilView);

    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode    = D3D11_FILL_SOLID;
    rd.CullMode    = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;
    
    dev->CreateRasterizerState(&rd, &pRS);
    devcon->RSSetState(pRS);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = SCREEN_WIDTH;
    viewport.Height = SCREEN_HEIGHT;

    devcon->RSSetViewports(1, &viewport);

    // Create Depth‑Stencil Texture
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width              = SCREEN_WIDTH;
    descDepth.Height             = SCREEN_HEIGHT;
    descDepth.MipLevels          = 1;
    descDepth.ArraySize          = 1;
    descDepth.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count   = 4;  
    descDepth.Usage              = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags          = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthStencilBuffer = nullptr;
    dev->CreateTexture2D(&descDepth, nullptr, &depthStencilBuffer);

    // Create Depth‑Stencil View
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format             = descDepth.Format;
    descDSV.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    descDSV.Texture2D.MipSlice = 0;
    
    dev->CreateDepthStencilView(depthStencilBuffer, &descDSV, &depthStencilView);
    depthStencilBuffer->Release();

    // Bind render‑target AND depth‑stencil view
    devcon->OMSetRenderTargets(1, &backbuffer, depthStencilView);

    // Create & set Depth‑Stencil State
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable    = FALSE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc      = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable  = FALSE;

    D3D11_RASTERIZER_DESC wfDesc = {};
    wfDesc.FillMode         = D3D11_FILL_WIREFRAME;
    wfDesc.CullMode         = D3D11_CULL_NONE;
    wfDesc.DepthClipEnable  = TRUE;
    ID3D11RasterizerState* wfRS = nullptr;
    
    dev->CreateRasterizerState(&wfDesc, &wfRS);
    devcon->RSSetState(wfRS);

    dev->CreateDepthStencilState(&dsDesc, &depthStencilState);
    devcon->OMSetDepthStencilState(depthStencilState, 0);

    InitPipeline();
    InitMatrix();
    InitGraphics();
}

void CleanD3D(void)
{
    swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

    // close and release all existing COM objects
    pLayout->Release();
    pVS->Release();
    pPS->Release();
    pVBuffer->Release();
    swapchain->Release();
    backbuffer->Release();
    dev->Release();
    devcon->Release();
    depthStencilState->Release();
    depthStencilView->Release();
    indexBuffer->Release();
    matrixBuffer->Release();
}

// this is the function used to render a single frame
void RenderFrame(void)
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    LONGLONG deltaTicks = currentTime.QuadPart - gPrevTime.QuadPart;
    gPrevTime = currentTime;
    float deltaTime = float(deltaTicks) / float(gTimerFreq.QuadPart);

    // clear the back buffer to a deep blue
    const FLOAT clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, clearColor);
    devcon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // select which vertex buffer to display
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
    devcon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    
    // select which primtive type we are using
    devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    XMMATRIX modelMatrix = XMMatrixIdentity();
    XMMATRIX viewMatrix = XMMatrixLookAtLH(
        XMVectorSet(0.0f, 5.0f, -15.0f, 1.0f),  // Camera Position
        XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),   // LookAt Target
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));  // Up Vector
    
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(30.0f),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,  // aspect ratio
        0.1f,
        100.0f);
    
    gRotationAngle += gRotationSpeed * deltaTime;
    if (gRotationAngle > XM_2PI)
        gRotationAngle -= XM_2PI;
    
    modelMatrix = XMMatrixRotationY(gRotationAngle);
    
    MatrixBufferType matrices;
    matrices.model = XMMatrixTranspose(modelMatrix);
    matrices.view = XMMatrixTranspose(viewMatrix);
    matrices.projection = XMMatrixTranspose(projectionMatrix);
    devcon->UpdateSubresource(matrixBuffer, 0, nullptr, &matrices, 0, 0);
    devcon->VSSetConstantBuffers(0, 1, &matrixBuffer);
     
    // draw the vertex buffer to the back buffer
    devcon->DrawIndexed(18, 0, 0);

    // switch the back buffer and the front buffer
    swapchain->Present(0, 0);
}

// this is the function that creates the shape to render
void InitGraphics()
{
    // create a triangle using the VERTEX struct
    // VERTEX OurVertices[] =
    // {
    //     {0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    //     {0.45f, -0.5, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    //     {-0.45f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f}
    // };

    VERTEX vertices[] = 
    {
        // position          //color
        // BASE
        { -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
        {  1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
        {  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 1.0f },
        { -1.0f, 0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
        // TOP
        {  0.0f, 1.5f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f }
    };
    
    DWORD indices[] = 
    {
        // base
        0, 2, 1,
        0, 3, 2,
    
        // sides
        0, 4, 1,
        1, 4, 2,
        2, 4, 3,
        3, 4, 0
    };


    // create the vertex buffer
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));

    vbd.Usage = D3D11_USAGE_DYNAMIC;                                  // write access access by CPU and GPU
    vbd.ByteWidth = sizeof(VERTEX) * ARRAYSIZE(vertices);             // size is the VERTEX struct * size of vertices array
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;                         // use as a vertex buffer
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;                      // allow CPU to write in buffer
                     
    dev->CreateBuffer(&vbd, NULL, &pVBuffer);                         // create the buffer

    // create the index buffer
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(DWORD) * ARRAYSIZE(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA initData = { indices };
    initData.pSysMem = indices;

    HRESULT hr = dev->CreateBuffer(&ibd, &initData, &indexBuffer);
    if (FAILED(hr))
        MessageBox(nullptr, L"FAILED TO CreateBuffer", L"InitiGraphics", MB_ICONERROR);
    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, vertices, sizeof(vertices));                       // copy the data
    devcon->Unmap(pVBuffer, NULL);                                      // unmap the buffer
}

void InitMatrix()
{
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(MatrixBufferType);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    dev->CreateBuffer(&cbd, nullptr, &matrixBuffer);
}

// this function loads and prepares the shaders
void InitPipeline()
{

    // load and compile the two shaders
    ID3DBlob *VS = nullptr, *PS = nullptr;
	HRESULT hr;
    
    hr = D3DCompileFromFile(
    						L"shaders.shader",
    						nullptr,
    						D3D_COMPILE_STANDARD_FILE_INCLUDE,
    						"VShader",
    						"vs_4_0",
    						0, 0,
    						&VS,
    						nullptr);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Errore compilazione vertex shader", L"InitPipeline", MB_ICONERROR);
        PostQuitMessage(1);
        return;
    }

    hr = D3DCompileFromFile(
    						L"shaders.shader",
    						nullptr,
    						D3D_COMPILE_STANDARD_FILE_INCLUDE,
    						"PShader",
    						"ps_4_0",
    						0, 0,
    						&PS,
    						nullptr);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Errore compilazione pixel shader", L"InitPipeline", MB_ICONERROR);
        PostQuitMessage(1);
        return;
    }
    // encapsulate both shaders into shader objects
    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

    // set the shader objects
    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);

    // create the input layout object
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
    devcon->IASetInputLayout(pLayout);
}