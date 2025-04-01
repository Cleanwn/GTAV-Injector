#pragma once

// Only Wide
bool FilePickerW(LPCWSTR title, LPCWSTR filter, DWORD filter_index, fs::path* out_path = nullptr, const char** out_err = nullptr);
bool FilePickerMultiW(LPCWSTR title, LPCWSTR filter, DWORD filter_index, std::vector<fs::path>* out_paths = nullptr, const char** out_err = nullptr);


std::wstring ReadFileW(const fs::path& path);
std::string ReadFileA(const fs::path& path);

#ifdef UNICODE
#define ReadFile ReadFileW
#else
#define ReadFile ReadFileA
#endif


void WriteFileW(const fs::path& path, const std::wstring& data);
void WriteFileA(const fs::path& path, const std::string& data);

#ifdef UNICODE
#define WriteFile WriteFileW
#else
#define WriteFile WriteFileA
#endif


DWORD CopyToClipboardW(const std::wstring& str);
DWORD CopyToClipboardA(const std::string& str);

#ifdef UNICODE
#define CopyToClipboard CopyToClipboardW
#else
#define CopyToClipboard CopyToClipboardA
#endif

void LoadSettings();
void SaveSettings();
