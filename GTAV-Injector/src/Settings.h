#pragma once

namespace Settings
{
	enum GameType : int
	{
		Legacy,
		Enhanced
	};

	inline GameType game_type = Legacy;

	enum LauncherType : int
	{
		Rockstar,
		EpicGames,
		Steam
	};

	inline LauncherType launcher_type = Rockstar;

	struct DllEntry
	{
		bool enabled;
		fs::path path;
	};

	inline std::vector<DllEntry> dll_entries;
}