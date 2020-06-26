#pragma once

#include "resource.h"

//#include "pch.h"

//extern HMODULE hMod;

wchar_t** get_host(void);
wchar_t** get_port();
void init_dialog(HWND hwnd, HMODULE hmod);
void get_hwnd_current_proc(HWND& hwndfind);