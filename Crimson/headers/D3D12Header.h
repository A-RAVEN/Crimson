#pragma once
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX
#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include "d3dx12.h"
#endif