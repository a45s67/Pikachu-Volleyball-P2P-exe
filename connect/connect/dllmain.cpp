// dllmain.cpp : 定義 DLL 應用程式的進入點。
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include<Windows.h>
#include "pch.h"
#include "connect.h"
#include "dialog.h"
#include "atlbase.h"
#include "atlstr.h"
#include "comutil.h"

HMODULE hMod;
//#include "dialog.cpp"

//char* get_host(void);
/*
typedef struct addrinfo
{
    int                 ai_flags;       // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
    int                 ai_family;      // PF_xxx
    int                 ai_socktype;    // SOCK_xxx
    int                 ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6
    size_t              ai_addrlen;     // Length of ai_addr
    char *              ai_canonname;   // Canonical name for nodename
    _Field_size_bytes_(ai_addrlen) struct sockaddr *   ai_addr;        // Binary address
    struct addrinfo *   ai_next;        // Next structure in linked list
}*/
SOCKET Connectsocket = INVALID_SOCKET;
int socket_exist() {
    return Connectsocket!= INVALID_SOCKET;
}
void send_wrap(char* buf, int size) {
    int iresult;
    iresult = send(Connectsocket, buf, size, 0);

}
void recv_wrap(char* buf, int size) {
    int iresult = recv(Connectsocket, buf, size, 0);
}

void set_client(struct addrinfo* result) {
    struct addrinfo*  ptr = result;
    int iresult;

    for (ptr = result;ptr != NULL;ptr = ptr->ai_next) {
        Connectsocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (Connectsocket == INVALID_SOCKET) {
            printf("Create socket failed.\n");
            WSACleanup();
            return;
        }

        iresult = connect(Connectsocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iresult == SOCKET_ERROR) {
            closesocket(Connectsocket);
            Connectsocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    if (Connectsocket == INVALID_SOCKET) {
        printf("Unable to connect to server! code-%d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    printf("connect success\n");
}

void set_server(struct addrinfo* result) {

    int iresult = bind(Connectsocket, result->ai_addr, (int)result->ai_addrlen);
    if (iresult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(Connectsocket);
        WSACleanup();
        return;
    }
    freeaddrinfo(result);
    if (listen(Connectsocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(Connectsocket);
        WSACleanup();
        return;
    }

    SOCKET tmpsoc;
    tmpsoc = accept(Connectsocket, NULL, NULL);
    if (tmpsoc == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(tmpsoc);
        WSACleanup();
        return;
    }

    closesocket(Connectsocket);
    printf("get client\n");
    Connectsocket = tmpsoc;
}



int set_sever_client() {
    int instruction = MessageBox(NULL,
        L"Create a server? (press NO will try to connect to the other)",
        L"server or client?",
        MB_YESNOCANCEL);
    wchar_t** buf;

    if (instruction == IDNO) {
        
        //wchar_t szValidDate = L"Hello World."; 
        //wcstombs(szTime, szValidDate, wcslen(szTime));
        char c1[0x40];
        char c2[0x40];
        buf = get_host();
        int wl0 = wcslen(buf[0])+1; 
        int wl1 = wcslen(buf[1]) + 1;

        size_t convertedChars = 0;

        wcstombs_s(&convertedChars, c1, wl0, buf[0], _TRUNCATE);
        convertedChars = 0;
        wcstombs_s(&convertedChars, c2, wl1, buf[1], _TRUNCATE);
        //wcstombs(c,get_host(), wcslen(c));
        printf("connect to %s:%s", c1,c2);
        create_socket(c1,c2);//get_host()
        return 2;
    }
    else if(instruction==IDYES){
        buf = get_port();

        char c[0x40];
        int wl = wcslen(buf[1]) + 1;
        size_t convertedChars = 0;
        wcstombs_s(&convertedChars, c, wl, buf[1], _TRUNCATE);

        create_socket(0,c);
        return 1;
    }
    else {
        return 0;
    }

}

void create_socket(char* host,char* port) {

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (host == 0) {
        hints.ai_flags = AI_PASSIVE;
        printf("passive\n");
    }

    int iresult = getaddrinfo(host, port, &hints, &result);
    printf("getaddrifo:%d\n", iresult);
    ptr = result;
    Connectsocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (Connectsocket == INVALID_SOCKET) {

        printf("Create socket failed: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return;
    }
    //SOCKET Connectsocket = INVALID_SOCKET;
    if (host == 0) {
        set_server(result);
    }
    else {
        set_client(result);
    }

}


DWORD WINAPI Threadshow(LPVOID lpParameter) {
//#define MAX_PATH 0x100
    
    wchar_t szPath[MAX_PATH];
    wchar_t szBuf[1024] = { 0 };
    GetModuleFileName(NULL, szPath, MAX_PATH);
    wsprintf(szBuf, L"DLL 注入 %s , pid=%d",szPath,GetCurrentProcessId());
    MessageBox(NULL, szBuf, L"DLL inject", MB_OK);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{

    WSADATA wsaData;
    int iresult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iresult != 0) {
        printf("start up failed.\n");
    }

    hMod = hModule;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, Threadshow, NULL, 0, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

