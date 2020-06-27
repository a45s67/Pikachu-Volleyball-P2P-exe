#pragma once
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "connect.h"
#include "resource.h"
#include <Commctrl.h>
#include <Shellapi.h>
void search_room();
void init_interact(HWND hwnd, HMODULE hmod, SOCKET &socket);
void send_verbose(char* c, int size);
void recv_verbose(char* c, int size);
//void close_socket(SOCKET &Connectsocket);

void interact_diag();

//void close_socket(SOCKET* socket);

bool play_again();
bool set_pos(int &pos);
//void chat_room();

