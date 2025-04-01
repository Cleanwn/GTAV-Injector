#include "Settings.h"
#include "Helpers/d3d11Helper.h"
#include "Helpers/GTAV_Helper.h"
#include "Helpers/HelperFunctions.h"
#include "Helpers/ImGuiHelper.h"
#include "Helpers/WindowsHelper.h"

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RenderFrame(bool vsync);


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int)
{
	LoadSettings();
	g_instance = hInstance;

	if (DWORD c = RegisterClassH(g_instance, APP_CLASS_NAME, WndProc); c != 0)
	{
		MessageBox(nullptr, std::format("failed to register class ({})", c).c_str(), "Error", MB_OK | MB_ICONERROR);
		return (int)c;
	}

	if (DWORD c = CreateWindowH(&g_main_hwnd, g_instance, APP_NAME, APP_CLASS_NAME, APP_WIDTH, APP_HEIGHT, 0, CW_USEDEFAULT, CW_USEDEFAULT); c != 0)
	{
		MessageBox(nullptr, std::format("failed to create window ({})", c).c_str(), "Error", MB_OK | MB_ICONERROR);
		UnregisterClassH(g_instance, APP_CLASS_NAME);
		return (int)c;
	}

	if (HRESULT hr = InitD3D11(g_main_hwnd, APP_WIDTH, APP_HEIGHT); FAILED(hr))
	{
		MessageBox(g_main_hwnd, std::format("failed to initialize D3D11 ({})", hr).c_str(), "Error", MB_OK | MB_ICONERROR);
		CleanupD3D11();
		DestroyWindowH(g_main_hwnd);
		UnregisterClassH(g_instance, APP_CLASS_NAME);
		return (int)hr;
	}

	if (!InitImGui(g_main_hwnd))
	{
		MessageBox(nullptr, "failed to initialize ImGui", "Error", MB_OK | MB_ICONERROR);
		CleanupD3D11();
		DestroyWindowH(g_main_hwnd);
		UnregisterClassH(g_instance, APP_CLASS_NAME);
		return 1;
	}

	ShowWindow(g_main_hwnd, SW_SHOWDEFAULT);
	UpdateWindow(g_main_hwnd);
	g_running = true;

	while (g_running)
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				g_running = false;
				break;
			}
		}

		if (!g_running)
			break;

		RenderFrame(true);
	}

	CleanupImGui();
	CleanupD3D11();
	if (g_main_hwnd)
		DestroyWindowH(g_main_hwnd);
	UnregisterClassH(g_instance, APP_CLASS_NAME);

	return 0;
}

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
		case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)
				return 0;

			g_resize_width = (UINT)LOWORD(lParam);
			g_resize_height = (UINT)HIWORD(lParam);
			return 0;
		}
		case WM_SYSCOMMAND:
		{
			if ((wParam & 0xfff0) == SC_KEYMENU)
				return 0;

			break;
		}
		case WM_CLOSE:
		{
			if (hWnd == g_main_hwnd)
			{
				g_running = false;
				DestroyWindowH(g_main_hwnd);
			}
			else DestroyWindowH(hWnd);

			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


size_t clicked_index = (size_t)-1;
void DllContextMenu(Settings::DllEntry& entry)
{
	if (clicked_index != (size_t)-1 && ImGui::BeginPopupContextItem("ContextMenu"))
	{
		ImGui::Text("%s", entry.path.string().c_str());
		ImGui::Separator();

		if (ImGui::Button("Open Path"))
		{
			ShellExecuteW(nullptr, L"open", L"explorer", (L"/select,\"" + entry.path.wstring() + L"\"").c_str(), nullptr, SW_SHOWDEFAULT);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Copy Path"))
		{
			CopyToClipboardW(entry.path.wstring());
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			Settings::dll_entries.erase(Settings::dll_entries.begin() + (int64_t)clicked_index);
			SaveSettings();
			ImGui::CloseCurrentPopup();
		}

		if (clicked_index == 0) ImGui::BeginDisabled();
		ImGui::Separator();
		if (ImGui::Button("Move Up"))
		{
			if (clicked_index != 0)
			{
				Settings::DllEntry copy = Settings::dll_entries[clicked_index];
				Settings::dll_entries.erase(Settings::dll_entries.begin() + (int64_t)clicked_index);
				Settings::dll_entries.insert(Settings::dll_entries.begin() + (int64_t)clicked_index - 1, copy);
			}
			SaveSettings();
			ImGui::CloseCurrentPopup();
		}
		if (clicked_index == 0) ImGui::EndDisabled();

		if (clicked_index == Settings::dll_entries.size() - 1) ImGui::BeginDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Move Down"))
		{
			if (clicked_index != Settings::dll_entries.size() - 1)
			{
				Settings::DllEntry copy = Settings::dll_entries[clicked_index];
				Settings::dll_entries.erase(Settings::dll_entries.begin() + (int64_t)clicked_index);
				Settings::dll_entries.insert(Settings::dll_entries.begin() + (int64_t)clicked_index + 1, copy);
			}
			SaveSettings();
			ImGui::CloseCurrentPopup();
		}
		if (clicked_index == Settings::dll_entries.size() - 1) ImGui::EndDisabled();

		ImGui::EndPopup();
	}
}

void RenderFrame(bool vsync)
{
	if (g_resize_width != 0 && g_resize_height != 0)
	{
		if (g_render_target_view)
		{
			g_render_target_view->Release();
			g_render_target_view = nullptr;
		}

		g_swap_chain->ResizeBuffers(0, g_resize_width, g_resize_height, DXGI_FORMAT_UNKNOWN, 0);
		g_resize_width = 0;
		g_resize_height = 0;

		ID3D11Texture2D* pBackBuffer;
		g_swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		g_device->CreateRenderTargetView(pBackBuffer, nullptr, &g_render_target_view);
		pBackBuffer->Release();
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	{
		// Right Side
		{
			ImGui::SetNextWindowPos({ APP_WIDTH - 200.f, 0.f });
			ImGui::SetNextWindowSize({ 200.f, 100.f });
			if (ImGui::Begin("Injector Game Type", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Game Type");
				if (ImGui::RadioButton("Legacy", (int*)&Settings::game_type, Settings::Legacy))
				{
					Settings::game_type = Settings::Legacy;
					IsGameRunning(true);
					SaveSettings();
				}

				if (ImGui::RadioButton("Enhanced", (int*)&Settings::game_type, Settings::Enhanced))
				{
					Settings::game_type = Settings::Enhanced;
					IsGameRunning(true);
					SaveSettings();
				}
			}
			ImGui::End();

			ImGui::SetNextWindowPos({ APP_WIDTH - 200.f,100.f });
			ImGui::SetNextWindowSize({ 200.f, APP_HEIGHT - 100.f });
			if (ImGui::Begin("Injector Launcher Type", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Launcher");
				if (ImGui::RadioButton("Rockstar", (int*)&Settings::launcher_type, Settings::Rockstar))
				{
					Settings::launcher_type = Settings::Rockstar;
					SaveSettings();
				}

				if (ImGui::RadioButton("Epic Games", (int*)&Settings::launcher_type, Settings::EpicGames))
				{
					Settings::launcher_type = Settings::EpicGames;
					SaveSettings();
				}

				if (ImGui::RadioButton("Steam", (int*)&Settings::launcher_type, Settings::Steam))
				{
					Settings::launcher_type = Settings::Steam;
					SaveSettings();
				}
			}
			ImGui::End();
		}

		// Left Side
		{
			ImGui::SetNextWindowPos({ 0.f, 0.f });
			ImGui::SetNextWindowSize({ APP_WIDTH - 200.f, APP_HEIGHT });
			if (ImGui::Begin("Injector Launch", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				bool is_game_running = IsGameRunning(false);
				if (!is_game_running)
				{
					if (ImGui::Button("Launch Game"))
					{
						LaunchGame();
					}
				}
				else
				{
					bool is_window_visible = IsWindowVisible();
					if (ImGui::Button(is_window_visible ? "Inject" : "Inject (Window not Visible)"))
					{
						try
						{
							InjectDLLs();
						}
						catch (std::exception& e)
						{
							MessageBox(g_main_hwnd, e.what(), "Warning", MB_OK | MB_ICONWARNING);
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Kill Game"))
					{
						KillGame();
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Add DLL"))
				{
					if (std::vector<fs::path> paths; FilePickerMultiW(L"Pick one or more DLL files", L"All Files (*.*)\0*.*\0DLL (*.dll)\0*.dll\0", 2, &paths))
					{
						for (const fs::path& path : paths)
						{
							if (!is_regular_file(path))
								continue;

							if (!std::ranges::any_of(Settings::dll_entries, [path](Settings::DllEntry& entry) { return entry.path == path; }))
								Settings::dll_entries.emplace_back(true, path);
						}
						SaveSettings();
					}
				}

				ImGui::Separator();
				if (ImGui::BeginTable("DLLs", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, { 0.f, ImGui::GetMainViewport()->Size.y - ImGui::GetCursorPos().y - 10.f }))
				{
					ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, 20.f);
					ImGui::TableSetupColumn("DLL Name", ImGuiTableColumnFlags_WidthFixed, 150.f);
					ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableHeadersRow();

					for (size_t i = 0; i < Settings::dll_entries.size(); ++i)
					{
						Settings::DllEntry& entry = Settings::dll_entries[i];
						if (i == clicked_index)
						{
							DllContextMenu(entry);
						}

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.f, 3.f));
						if (ImGui::Checkbox(std::format("##Enabled{}", i).c_str(), &entry.enabled))
						{
							SaveSettings();
						}
						ImGui::PopStyleVar();

						ImGui::TableNextColumn();
						if (ImGui::Selectable(entry.path.filename().string().c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
						{
							clicked_index = i;
							ImGui::OpenPopup("ContextMenu");
						}

						ImGui::TableNextColumn();
						if (ImGui::Selectable(entry.path.string().c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
						{
							clicked_index = i;
							ImGui::OpenPopup("ContextMenu");
						}
					}
					ImGui::EndTable();
				}
			}
			ImGui::End();
		}
	}

	ImGui::Render();
	constexpr float clear_color[4] = { 0.f, 0.f, 0.f, 1.f };
	g_context->OMSetRenderTargets(1, &g_render_target_view, nullptr);
	g_context->ClearRenderTargetView(g_render_target_view, clear_color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_swap_chain->Present(vsync ? 1 : 0, 0);
}
