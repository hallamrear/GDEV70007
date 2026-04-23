// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define NOMINMAX

// add headers that you want to pre-compile here
#include "framework.h"

//RPC Library for GUIDs
#pragma comment(lib, "Rpcrt4.lib")
#include <rpc.h>

//DirectX XInput
#define XINPUT_ON_GAMEINPUT_NO_XINPUTENABLE
#include <XInput.h>
#pragma comment(lib,"xinput.lib")

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ Standard Libraries
#include <vector>
#include <string>
#include <filesystem>
#include <cassert>
#include <fstream>

//#define RENDERER_DX11
#define RENDERER_DX12

#include <System/Defines.h>
#include <System/Types.h>
#include <System/ServiceLocator.h>

#endif //PCH_H
