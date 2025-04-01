#pragma once

DWORD RegisterClassH(HINSTANCE instance, const char* class_name, WNDPROC wndproc);
DWORD UnregisterClassH(HINSTANCE instance, const char* class_name);

DWORD CreateWindowH(HWND* out_hwnd, HINSTANCE instance, const char* window_name, const char* class_name, int width, int height, DWORD ex_style = 0, int pos_x = CW_USEDEFAULT, int pos_y = CW_USEDEFAULT, HWND parent = nullptr);
DWORD DestroyWindowH(HWND& hwnd);
