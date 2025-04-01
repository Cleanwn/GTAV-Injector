#pragma once

#include <Windows.h>

#include <Shellapi.h>
#include <tlhelp32.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgi.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#define MINI_CASE_SENSITIVE
#include <ini.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
