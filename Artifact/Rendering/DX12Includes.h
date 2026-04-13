#pragma once

//Link DX12 Libraries
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#include <DirectX/d3d12.h>
#include <DirectX/d3dx12.h>
#include <DirectX/d3dcommon.h>
#include <DirectX/d3d12compiler.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <winerror.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

//DX12 IMGUI Libraries
#include "Rendering/IMGUIIncludes.h"
#include "Rendering/imgui/backends/imgui_impl_dx12.h"