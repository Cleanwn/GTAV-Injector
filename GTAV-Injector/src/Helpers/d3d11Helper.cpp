#include "d3d11Helper.h"

HRESULT InitD3D11(HWND hwnd, unsigned width, unsigned height, unsigned refresh_rate)
{
	DXGI_SWAP_CHAIN_DESC desc = {};
	desc.BufferDesc.Width = width;
	desc.BufferDesc.Height = height;
	desc.BufferDesc.RefreshRate.Numerator = refresh_rate;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.OutputWindow = hwnd;
	desc.Windowed = true;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	constexpr D3D_FEATURE_LEVEL feature_levels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		feature_levels,
		std::size(feature_levels),
		D3D11_SDK_VERSION,
		&desc,
		&g_swap_chain,
		&g_device,
		nullptr,
		&g_context
	);

	if (hr == DXGI_ERROR_UNSUPPORTED)
	{
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			nullptr,
			0,
			feature_levels,
			std::size(feature_levels),
			D3D11_SDK_VERSION,
			&desc,
			&g_swap_chain,
			&g_device,
			nullptr,
			&g_context
		);
	}

	return hr;
}

void CleanupD3D11()
{
	if (g_swap_chain != nullptr) { g_swap_chain->Release(); g_swap_chain = nullptr; }
	if (g_device != nullptr) { g_device->Release(); g_device = nullptr; }
	if (g_context != nullptr) { g_context->Release(); g_context = nullptr; }
	if (g_render_target_view != nullptr) { g_render_target_view->Release(); g_render_target_view = nullptr; }
}
