#include <dwmapi.h>

#include "Windows.h"
#include "Resource/resource.h"

DWORD RegisterClassH(HINSTANCE instance, const char* class_name, WNDPROC wndproc)
{
	WNDCLASSEXA wc = { };
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.lpfnWndProc = wndproc;
	wc.hInstance = instance;
	wc.hIcon = LoadIconA(instance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hIconSm = LoadIconA(instance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
	wc.lpszClassName = class_name;
	if (RegisterClassExA(&wc) == 0)
		return GetLastError();

	return 0;
}

DWORD UnregisterClassH(HINSTANCE instance, const char* class_name)
{
	if (!UnregisterClassA(class_name, instance))
		return GetLastError();

	return 0;
}


DWORD CreateWindowH(HWND* out_hwnd, HINSTANCE instance, const char* window_name, const char* class_name, int width, int height, DWORD ex_style, int pos_x, int pos_y, HWND parent)
{
	*out_hwnd = CreateWindowExA(ex_style, class_name, window_name, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, pos_x, pos_y, width, height, parent, nullptr, instance, nullptr);
	BOOL dark = true;
	DwmSetWindowAttribute(*out_hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
	if (*out_hwnd == nullptr)
		return GetLastError();

	return 0;
}

DWORD DestroyWindowH(HWND& hwnd)
{
	if (!DestroyWindow(hwnd))
	{
		return GetLastError();
	}

	hwnd = nullptr;
	return 0;
}
