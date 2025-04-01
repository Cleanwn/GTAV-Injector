#pragma once

HRESULT InitD3D11(HWND hwnd, unsigned width, unsigned height, unsigned refresh_rate = 60);
void CleanupD3D11();
