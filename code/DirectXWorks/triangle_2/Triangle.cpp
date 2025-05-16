#include "pch.h"
#include "Trinagle.h"

// this function initializes and prepares Direct3D for use
void CTriangle::Initialize()
{

    // Define temporary pointers to a device and a device context
    ComPtr<ID3D11Device> dev11;
    ComPtr<ID3D11DeviceContext> devcon11;

    // Create the device and device context objects
    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &dev11,
        nullptr,
        &devcon11);
    
    // Convert the pointers from the DirectX 11 versions to the DirectX 11.1 versions
    dev11.As(&dev);
    devcon11.As(&devcon);
}

// this function performs updates to the state of the game
void CTriangle::Update()
{
}

// this function renders a single frame of 3D graphics
void CTriangle::Render()
{
}