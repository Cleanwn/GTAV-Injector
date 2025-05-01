#include "GTAV_Helper.h"

#include "Settings.h"

bool ReadStringFromRegistry(const std::wstring& reg_sub_key, const std::wstring& reg_value, std::wstring* out_str)
{
	size_t buffer_size = MAX_PATH;
	std::wstring buffer;
	buffer.resize(buffer_size);
	DWORD cb_data = (DWORD)(buffer_size * sizeof(wchar_t));

	LSTATUS status = RegGetValueW(
		HKEY_LOCAL_MACHINE,
		reg_sub_key.c_str(),
		reg_value.c_str(),
		RRF_RT_REG_SZ,
		nullptr,
		buffer.data(),
		&cb_data
	);

	while (status == ERROR_MORE_DATA)
	{
		cb_data /= sizeof(wchar_t);
		if (cb_data > (DWORD)buffer_size)
		{
			buffer_size = (size_t)cb_data;
		}
		else
		{
			buffer_size *= 2;
			cb_data = (DWORD)buffer_size * sizeof(wchar_t);
		}

		buffer.resize(buffer_size);
		status = RegGetValueW(
			HKEY_LOCAL_MACHINE,
			reg_sub_key.c_str(),
			reg_value.c_str(),
			RRF_RT_REG_SZ,
			nullptr,
			buffer.data(),
			&cb_data
		);
	}

	if (status == ERROR_SUCCESS)
	{
		cb_data /= sizeof(wchar_t);
		buffer.resize(cb_data - 1);

		*out_str = buffer;
		return true;
	}

	return false;
}

bool GetPid(DWORD* out_pid)
{
	std::string_view process_name = Settings::game_type == Settings::Legacy ? "GTA5.exe" : "GTA5_Enhanced.exe";

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32 pe = {};
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snapshot, &pe))
	{
		do
		{
			if (process_name == pe.szExeFile)
			{
				if (out_pid)
					*out_pid = pe.th32ProcessID;

				return true;
			}
		}
		while (Process32Next(snapshot, &pe));
	}

	CloseHandle(snapshot);
	return false;
}

bool last_check_result = false;
uint64_t last_check = 0;

bool IsGameRunning(bool no_time)
{
	if (last_check == 0 || no_time || GetTickCount64() - last_check > 3'000)
	{
		last_check_result = GetPid(nullptr);
		last_check = GetTickCount64();
	}

	return last_check_result;
}

bool IsWindowVisible()
{
	return FindWindowA(Settings::game_type == Settings::Legacy ? "grcWindow" : "sgaWindow", nullptr) != nullptr;
}

void LaunchGame()
{
	if (IsGameRunning(true))
	{
		MessageBoxA(g_main_hwnd, "Game is already running", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	switch (Settings::launcher_type)
	{
		case Settings::Rockstar:
		{
			if (std::wstring path; ReadStringFromRegistry(Settings::game_type == Settings::Legacy ? L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Grand Theft Auto V" : L"SOFTWARE\\WOW6432Node\\Rockstar Games\\GTAV Enhanced", L"InstallFolder", &path))
			{
				ShellExecuteW(nullptr, L"open", (path + L"\\PlayGTAV.exe").c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			}
			break;
		}
		case Settings::EpicGames:
		{
			if (Settings::game_type == Settings::Legacy)
				ShellExecuteA(nullptr, "open", "com.epicgames.launcher://apps/9d2d0eb64d5c44529cece33fe2a46482?action=launch&silent=true", nullptr, nullptr, SW_SHOWNORMAL);
			else
				ShellExecuteA(nullptr, "open", "com.epicgames.launcher://apps/8769e24080ea413b8ebca3f1b8c50951?action=launch&silent=true", nullptr, nullptr, SW_SHOWNORMAL);

			break;

		}
		case Settings::Steam:
		{
			if (Settings::game_type == Settings::Legacy)
				ShellExecuteA(nullptr, "open", "steam://run/271590", nullptr, nullptr, SW_SHOWNORMAL);
			else
				ShellExecuteA(nullptr, "open", "steam://run/3240220", nullptr, nullptr, SW_SHOWNORMAL);

			break;
		}
	}
}

void KillGame()
{
	if (!IsGameRunning(true))
	{
		MessageBoxA(g_main_hwnd, "Game is not running", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	if (DWORD pid; GetPid(&pid))
	{
		HANDLE handle = OpenProcess(PROCESS_TERMINATE, false, pid);
		TerminateProcess(handle, 0);
	}
}

void InjectDLLs()
{
	HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
	void* loadlibA = GetProcAddress(kernel32, "LoadLibraryA");
	if (loadlibA == nullptr)
		throw std::runtime_error("Failed to get LoadLibraryA");

	DWORD pid;
	if (!GetPid(&pid))
		throw std::runtime_error("Game is not running");

	HANDLE handle = OpenProcess(1082, 1, pid);
	if (handle == nullptr)
		throw std::runtime_error("Failed to get handle to game process");

	fs::path path = fs::path(std::getenv("appdata")) / APPDATA_FOLER / "Temp";  // NOLINT(concurrency-mt-unsafe)
	if (!exists(path))
	{
		create_directories(path);
	}
	else
	{
		for (const auto& file : fs::recursive_directory_iterator(path))
		{
			if (file.is_regular_file())
			{
				try
				{
					fs::remove(file);
				}
				catch (...) {}
			}
		}
	}

	size_t dll_count = 0;
	size_t injected_count = 0;
	for (Settings::DllEntry& entry : Settings::dll_entries)
	{
		BYTE* buffer = nullptr;
		try
		{
			if (!entry.enabled)
				continue;

			++dll_count;
			if (!exists(entry.path) || is_empty(entry.path))
			{
				MessageBoxA(g_main_hwnd, std::format("File '{}' does not exist or is empty", entry.path.string()).c_str(), "Warning", MB_OK | MB_ICONWARNING);
				continue;
			}

			fs::path file_path = path / entry.path.filename();
			if (exists(file_path))
			{
				size_t i = 0;
				std::wstring extension = file_path.extension();
				file_path.replace_extension("");
				do
				{
					++i;
				}
				while (fs::exists(file_path.wstring() + std::to_wstring(i) + extension));
				file_path = file_path.wstring() + L"_" + std::to_wstring(i) + extension;
			}

			copy_file(entry.path, file_path);

			size_t buffer_size = file_path.string().size() + 1;
			buffer = new BYTE[buffer_size];
			memcpy_s(buffer, buffer_size, file_path.string().data(), file_path.string().size());
			buffer[buffer_size - 1] = '\0';

			void* base = VirtualAllocEx(handle, nullptr, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if (base == nullptr)
				throw std::runtime_error(std::format("Failed to allocate {} bytes for '{}'", buffer_size, entry.path.string()));

			if (!WriteProcessMemory(handle, base, buffer, buffer_size, nullptr))
				throw std::runtime_error(std::format("Failed write memory for '{}'", entry.path.string()));

			if (CreateRemoteThread(handle, nullptr, 0, (LPTHREAD_START_ROUTINE)loadlibA, base, 0, nullptr) == nullptr)
				throw std::runtime_error(std::format("Failed create remote thread for '{}'", entry.path.string()));

			delete[] buffer;
			injected_count++;
		}
		catch (...)
		{
			delete[] buffer;
			CloseHandle(handle);
			throw;
		}
	}
	CloseHandle(handle);

	MessageBoxA(g_main_hwnd, std::format("Injected {}/{} DLLs", injected_count, dll_count).c_str(), "Injected", MB_OK | MB_ICONINFORMATION);
}
