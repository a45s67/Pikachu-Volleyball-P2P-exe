#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include<Windows.h>
#include "pch.h"
#include "connect.h"

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
void send_wrap(char *buf,int size) {
    int iresult;
    iresult = send(Connectsocket, buf, size, 0);

}
void recv_wrap(char* buf, int size) {
    int iresult = recv(Connectsocket, buf, size, 0);
}

void create_socket(char* host) {
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iresult = getaddrinfo(host,"8888", &hints, &result);
    printf("getaddrifo:%d\n", iresult);

    //SOCKET Connectsocket = INVALID_SOCKET;
    ptr = result;
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

}