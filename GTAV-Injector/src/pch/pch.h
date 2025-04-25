#pragma once
#include "framework.h"

#define APP_NAME "GTAV Injector"
#define APP_CLASS_NAME "GTAVINJECTOR"
#define APPDATA_FOLER "GTAV-Inj"
#define APP_WIDTH 750
#define APP_HEIGHT 280

inline bool g_running = false;
inline UINT g_resize_width = 0;
inline UINT g_resize_height = 0;

inline HINSTANCE g_instance;
inline HWND g_main_hwnd;

inline IDXGISwapChain* g_swap_chain;
inline ID3D11Device* g_device;
inline ID3D11DeviceContext* g_context;
inline ID3D11RenderTargetView* g_render_target_view;

namespace fs = std::filesystem;
