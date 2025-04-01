#pragma once

bool GetPid(DWORD* out_pid);
bool IsGameRunning(bool no_time);
bool IsWindowVisible();

void LaunchGame();
void KillGame();
void InjectDLLs();
