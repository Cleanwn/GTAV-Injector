#include "HelperFunctions.h"

#include "Settings.h"

bool FilePickerW(LPCWSTR title, LPCWSTR filter, DWORD filter_index, fs::path* out_path, const char** out_err)
{
	wchar_t buffer[MAX_PATH] = {};
	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_main_hwnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = filter_index;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = std::size(buffer);
	ofn.lpstrTitle = title;
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FORCESHOWHIDDEN | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameW(&ofn))
	{
		fs::path path = buffer;

		if (!exists(path))
		{
			if (out_err)
				*out_err = "File does not exist";

			return false;
		}

		if (out_path)
			*out_path = path;

		return true;
	}

	if (out_err)
	{
		switch (CommDlgExtendedError())
		{
			case CDERR_DIALOGFAILURE: *out_err = "failed to create dialog box"; break;
			case CDERR_FINDRESFAILURE: *out_err = "failed to find resource";  break;
			case CDERR_INITIALIZATION: *out_err = "failed dialog box initialization";  break;
			case CDERR_LOADRESFAILURE: *out_err = "failed to load resource";  break;
			case CDERR_LOADSTRFAILURE: *out_err = "failed to load string";  break;
			case CDERR_LOCKRESFAILURE: *out_err = "failed to lock resource";  break;
			case CDERR_MEMALLOCFAILURE: *out_err = "failed to allocate memory"; break;
			case CDERR_MEMLOCKFAILURE: *out_err = "failed to lock memory";  break;
			case CDERR_NOHINSTANCE: *out_err = "missing handle specified while ENABLETEMPLATE flag was set"; break;
			case CDERR_NOHOOK: *out_err = "missing hook procedure while ENABLEHOOK flag was set"; break;
			case CDERR_NOTEMPLATE: *out_err = "RegisterWindowMessage returned a error"; break;
			case CDERR_STRUCTSIZE: *out_err = "invalid struct size"; break;

			case FNERR_BUFFERTOOSMALL: *out_err = "file path size was bigger than buffer size";  break;
			case FNERR_INVALIDFILENAME: *out_err = "invalid file name"; break;
			case FNERR_SUBCLASSFAILURE: *out_err = "not enough memory"; break;
			default: *out_err = "dialog was canceled";
		}
	}

	return false;
}

bool FilePickerMultiW(LPCWSTR title, LPCWSTR filter, DWORD filter_index, std::vector<fs::path>* out_paths, const char** out_err)
{
	wchar_t buffer[2048] = {};
	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_main_hwnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = filter_index;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = std::size(buffer);
	ofn.lpstrTitle = title;
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FORCESHOWHIDDEN | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if (GetOpenFileNameW(&ofn))
	{
		fs::path path = buffer;
		size_t pos = path.wstring().size();

		if (out_paths)
		{
			while (true)
			{
				if (buffer[pos + 1] == '\0')
				{
					if (out_paths->empty())
					{
						out_paths->emplace_back(path);
						return true;
					}

					break;
				}

				std::wstring file_name;
				for (; buffer[pos + 1] != '\0'; pos++)
				{
					file_name += buffer[pos + 1];
				}

				out_paths->emplace_back(path.wstring() + L"\\" + file_name);
				pos++;
			}
		}

		return true;
	}

	if (out_err)
	{
		switch (CommDlgExtendedError())
		{
			case CDERR_DIALOGFAILURE: *out_err = "failed to create dialog box"; break;
			case CDERR_FINDRESFAILURE: *out_err = "failed to find resource";  break;
			case CDERR_INITIALIZATION: *out_err = "failed dialog box initialization";  break;
			case CDERR_LOADRESFAILURE: *out_err = "failed to load resource";  break;
			case CDERR_LOADSTRFAILURE: *out_err = "failed to load string";  break;
			case CDERR_LOCKRESFAILURE: *out_err = "failed to lock resource";  break;
			case CDERR_MEMALLOCFAILURE: *out_err = "failed to allocate memory"; break;
			case CDERR_MEMLOCKFAILURE: *out_err = "failed to lock memory";  break;
			case CDERR_NOHINSTANCE: *out_err = "missing handle specified while ENABLETEMPLATE flag was set"; break;
			case CDERR_NOHOOK: *out_err = "missing hook procedure while ENABLEHOOK flag was set"; break;
			case CDERR_NOTEMPLATE: *out_err = "RegisterWindowMessage returned a error"; break;
			case CDERR_STRUCTSIZE: *out_err = "invalid struct size"; break;

			case FNERR_BUFFERTOOSMALL: *out_err = "file path size was bigger than buffer size";  break;
			case FNERR_INVALIDFILENAME: *out_err = "invalid file name"; break;
			case FNERR_SUBCLASSFAILURE: *out_err = "not enough memory"; break;
			default: *out_err = "dialog was canceled";
		}
	}

	return false;
}


DWORD CopyToClipboardW(const std::wstring& str)
{
	if (!OpenClipboard(nullptr))
		return GetLastError();

	EmptyClipboard();

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (str.size() + 1) * sizeof(wchar_t));
	if (!hMem)
	{
		DWORD err = GetLastError();
		CloseClipboard();
		return err;
	}

	wchar_t* mem = static_cast<wchar_t*>(GlobalLock(hMem));
	if (!mem)
	{
		DWORD err = GetLastError();
		GlobalFree(hMem);
		CloseClipboard();
		return err;
	}

	wcscpy_s(mem, str.size() + 1, str.c_str());
	GlobalUnlock(hMem);
	SetClipboardData(CF_UNICODETEXT, hMem);
	CloseClipboard();
	return 0;
}

DWORD CopyToClipboardA(const std::string& str)
{
	if (!OpenClipboard(nullptr))
		return GetLastError();

	EmptyClipboard();

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, str.size() + 1);
	if (!hMem)
	{
		DWORD err = GetLastError();
		CloseClipboard();
		return err;
	}

	char* mem = static_cast<char*>(GlobalLock(hMem));
	if (!mem)
	{
		DWORD err = GetLastError();
		GlobalFree(hMem);
		CloseClipboard();
		return err;
	}

	memcpy(mem, str.c_str(), str.size() + 1);
	GlobalUnlock(hMem);
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
	return 0;
}


void LoadSettings()
{
	fs::path path = fs::path(std::getenv("userprofile")) / "Documents" / DOCUMENTS_FOLDER / "Settings.ini";  // NOLINT(concurrency-mt-unsafe)
	if (!exists(path.parent_path()))
	{
		create_directories(path.parent_path());
		return;
	}

	if (!exists(path))
		return;

	if (!is_regular_file(path))
	{
		fs::remove(path);
		return;
	}

	mINI::INIFile file(path);
	mINI::INIStructure ini;
	file.read(ini);

	try
	{
		Settings::game_type = (Settings::GameType)std::stoi(ini["Launcher"]["Game Type"]);
		Settings::launcher_type = (Settings::LauncherType)std::stoi(ini["Launcher"]["Launcher Type"]);

		for (auto& it : ini["DLLs"])
		{
			Settings::dll_entries.emplace_back(it.second == "1", it.first);
		}
	}
	catch (std::exception& e)
	{
		Settings::game_type = Settings::Legacy;
		Settings::launcher_type = Settings::Rockstar;
		Settings::dll_entries.clear();
		MessageBoxA(g_main_hwnd, e.what(), "Error Loading Settings", MB_OK | MB_ICONERROR);
	}
}

void SaveSettings()
{
	fs::path path = fs::path(std::getenv("userprofile")) / "Documents" / DOCUMENTS_FOLDER / "Settings.ini";  // NOLINT(concurrency-mt-unsafe)
	if (!exists(path.parent_path()))
		create_directories(path);

	mINI::INIFile file(path);
	mINI::INIStructure ini;

	ini["Launcher"]["Game Type"] = std::to_string(Settings::game_type);
	ini["Launcher"]["Launcher Type"] = std::to_string(Settings::launcher_type);

	for (Settings::DllEntry& entry : Settings::dll_entries)
	{
		ini["DLLs"][entry.path.string()] = entry.enabled ? "1" : "0";
	}

	bool _ = file.generate(ini, true);
}
