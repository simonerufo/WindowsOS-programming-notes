#pragma once

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Platform;
using namespace DirectX;

class CTriangle
{
public:
	ComPtr<ID3D11Device1> dev; // device interface
	ComPtr<ID3D11DeviceContext1> devcon; // decive context interface
	
	void Initialize();
	void Update();
	void Render();
}
