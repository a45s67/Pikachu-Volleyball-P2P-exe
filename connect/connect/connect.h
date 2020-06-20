#pragma once

#ifdef CONNECT_EXPORTS
#define CONNECT_API __declspec(dllexport)
#else
#define CONNECT_API __declspec(dllimport)
#endif

#include<WinSock2.h>
#include<WS2tcpip.h>
#include<stdio.h>
#pragma comment(lib,"Ws2_32.lib")

extern "C" CONNECT_API void recv_wrap(char* buf, int size);
extern "C" CONNECT_API void create_socket(char* host, char* port);
extern "C" CONNECT_API void send_wrap(char* buf, int size);
extern "C" CONNECT_API int socket_exist();
extern "C" CONNECT_API int set_sever_client();