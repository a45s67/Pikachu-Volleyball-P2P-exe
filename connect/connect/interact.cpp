#include "pch.h"
#include "interact.h"



static HWND hWnd = NULL;
static HMODULE hMod = NULL;
static SOCKET *Connectsocket = NULL;
static SOCKET Connectroomsocket = NULL;
class Room;

//bool create_room_socket(char* host, char* port);

void init_interact(HWND hwnd, HMODULE hmod, SOCKET &socket) {
    hWnd = hwnd;
    hMod = hmod;
    Connectsocket = &socket;
}

typedef struct Connect_infos_tag {

    char ip[0x10];
    char port[0x8];
    wchar_t name[0x20];
}Connect_infos;





Room* This;

class Room {
    
private:
    HANDLE ipportstrMutex;
    HANDLE searchMutex;
    HANDLE vecMutex;
    HWND hwndlist= NULL;
    bool searching = false;
    bool listening = false;
    std::vector<Connect_infos> rooms;

    Connect_infos myself;
    Connect_infos opponent;


public:
    Room()  {
        opponent = { 0 };
        myself = { 0 };
        searchMutex = CreateMutex(
            NULL,              // default security attributes
            FALSE,             // initially not owned
            NULL);
    }
    static DWORD WINAPI search_room_socket(LPVOID lpParameter);
    bool create_room_socket(const char* host, const char* port);
    //static INT_PTR CALLBACK search_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    //static INT_PTR CALLBACK create_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

    static DWORD WINAPI search_room(LPVOID lpParameter) {
        //Room* This = (Room*)lpParameter;
        HWND hwndlist = This->hwndlist;

        std::string host_base = "192.168.1.";
        std::string host;
        while (1) {

            #define THREADCOUNT 64

            HANDLE aThread[256];
            
            int dwWaitResult = WaitForSingleObject(
                This->searchMutex,    // handle to mutex
                6000);
            if (!This->searching)
                return 1;
            This->rooms.clear();

            DWORD ThreadID;
            for (int i = 0;i < 256;i++) {
                WaitForSingleObject(
                    This->ipportstrMutex,    // handle to mutex
                    300);


                Connect_infos* room = new Connect_infos();
                host = host_base + std::to_string(i);
                memcpy(room->ip, host.c_str() ,host.length());
                memcpy(room->port, "12345", 6);

                

                aThread[i] = CreateThread(
                    NULL,       // default security attributes
                    0,          // default stack size
                    (LPTHREAD_START_ROUTINE)search_room_socket,
                    room,       // no thread function arguments
                    0,          // default creation flags
                    &ThreadID); // receive thread identifier


                ReleaseMutex(This->ipportstrMutex);
                if (aThread[i] == NULL)
                {
                    printf("CreateThread error: %d, ip:%s\n", GetLastError(),room->ip);
                    //return 1;
                }
                /*
                if (This->search_room_socket(host.c_str(), "12345", room)) {
                    printf("i = %d\n", i);
                    This->rooms.push_back(room);
                }*/
            }
            for (int i = 0;i < (256 / THREADCOUNT);i++) {
                int res = WaitForMultipleObjects(THREADCOUNT, &(aThread[i* THREADCOUNT]), TRUE, INFINITE);
                if (res == WAIT_FAILED)
                    printf("wait failed\n");
            }
            ReleaseMutex(This->searchMutex);

            SendMessage(hwndlist, LB_RESETCONTENT, 0, 0);
            printf("MAXIMUM_WAIT_OBJECTS = %d", MAXIMUM_WAIT_OBJECTS);
            printf("room size = %d\n", This->rooms.size());
            for (unsigned int i = 0;i < This->rooms.size();i++) {
                /*
                char c[0x40];
                int wl = wcslen(buf[0]) + 1;
                size_t convertedChars = 0;
                wcstombs_s(&convertedChars, c1, wl0, buf[0], _TRUNCATE);
                */
                printf("add room : %s:%s, ", This->rooms[i].ip,This->rooms[i].port);
                wprintf(L"name = %s\n", This->rooms[i].name);

                int pos = SendMessageW(hwndlist, LB_ADDSTRING, 0, (LPARAM)This->rooms[i].name);
                SendMessageW(hwndlist, LB_SETITEMDATA, pos, (LPARAM)i);
            }
            
            Sleep(300);
        }
    }
    void search_dialog(HWND hwnd) {
        //printf("This=0x%x", this);
        DialogBoxParam(hMod, MAKEINTRESOURCE(IDD_DIALOG_SEARCHROOM), hwnd, search_proc,(LPARAM)this);
    }

    void create_dialog(HWND hwnd) {
        DialogBoxParam(hMod, MAKEINTRESOURCE(IDD_DIALOG_CREATEROOM), hwnd, create_proc, (LPARAM)this);
    }

    
    static DWORD WINAPI listen_wrap(LPVOID lpParameter) {

        if (!create_socket(0, This->myself.port)) {
            MessageBox(NULL, L"Match failed.\nCreate Room again.", L"Wrong", 0);
        }
            
        else
            MessageBox(NULL, L"Connect successfully.\nPlease turn to 2P mode.", L"Ya", 0);

        This->listening = false;
        Connect_infos *dummy = new Connect_infos{
            "127.0.0.1",
            "12345",L""
        };

        search_room_socket(dummy);
        return 1;
    }

    static DWORD WINAPI listen_room_wrap(LPVOID lpParameter) {
        //Room* This = (Room*)lpParameter;

        if (This->create_room_socket(0, "12345") == 0) {
            MessageBox(NULL, L"Failed to initialize room response", L"Room", 0);
            return 1;
        }

        MessageBox(NULL, L"Close room", L"Room", 0);
        return 1;
    }
    void create_room(HWND hDlg) {
        //char c[0x20];
        GetDlgItemTextW(hDlg, IDC_EDIT_ROOMNAME,(wchar_t*)myself.name, 0x20);//myself.name
        GetDlgItemTextA(hDlg, IDC_EDIT_ROOMPORT,(char*)myself.port, 0x8);//myself.port
        if (strlen(myself.port) < 2) {
            MessageBox(hDlg, L"Port must between 10~65535", L"Room", 0);
            return;
        }
        listening = true;
        if (CreateThread(NULL, 0x10, listen_wrap, this, 0, NULL) == NULL ||
            CreateThread(NULL, 0x10, listen_room_wrap, this, 0, NULL)==NULL) {
            MessageBox(hDlg, L"Create failed", L"Room", 0);
            return;
        }
        set_server();
        MessageBox(hDlg, L"Create successfully", L"Room", 0);

    }



    static INT_PTR CALLBACK search_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
        //printf("lParam=0x%x", lParam);
        //Sleep(3000);
        //Room* This = (Room*)lParam;
        switch (message)
        {
        case WM_INITDIALOG:
            This->hwndlist = GetDlgItem(hDlg, IDC_LIST_SEARCH);


            This->searching = true;
            CreateThread(NULL, 0x10, search_room, NULL, 0, NULL);
            Sleep(50);

            WaitForSingleObject(
                This->searchMutex,    // handle to mutex
                3000);
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            switch LOWORD(wParam) 
            {
            case IDOK:
                {

                if ( create_socket((char*)This->opponent.ip , (char*)This->opponent.port) == 0) {
                    MessageBox(NULL, L"Connect failed", L"Wrong", 0);
                }
                else {
                    MessageBox(NULL, L"Connect successfully.\nPlease turn to 2P mode.", L"Ya", 0);
                    This->searching = false;
                    EndDialog(hDlg, IDOK);
                }
                    //create_room();
                    return (INT_PTR)TRUE;
                }
            case IDC_BUTTON_UPDATE:
                {
                    ReleaseMutex(This->searchMutex);
                    // release the lock for search_room()
                    Sleep(10);
                    WaitForSingleObject(
                        This->searchMutex,    // handle to mutex
                        3000);
                    //create_room();
                    return (INT_PTR)TRUE;
                }
            case IDC_LIST_SEARCH:
            {
                switch HIWORD(wParam) {

                case LBN_SELCHANGE: {
                    printf("selchange param = %d\n", HIWORD(wParam));
                    WaitForSingleObject(This->searchMutex, INFINITE);


                    int lbitem = (int)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
                    int i = (int)SendMessage((HWND)lParam, LB_GETITEMDATA, lbitem, 0);
                    //This->hwndlist
                    printf("i = %d\n", i);
                    if (i == -1) {
                        return true;
                    }

                    char ip[0x20] = "IP:";
                    char port[0x10] = "Port:";
                    sprintf_s(ip,0x20, "IP\t:%s", This->rooms[i].ip);
                    sprintf_s(port,0x10, "Port\t:%s", This->rooms[i].port);


                    This->opponent = This->rooms[i];
                    SetDlgItemTextA(hDlg, IDC_STATIC_PORT, ip);
                    SetDlgItemTextA(hDlg, IDC_STATIC_IP, port);


                    ReleaseMutex(This->searchMutex);
                    return true;
                }   
                default:
                    printf("param = %d\n", HIWORD(wParam));
                }
                break;

            }
            case IDCANCEL:
                This->searching = false;
                EndDialog(hDlg, LOWORD(wParam));
                return true;
            }
        }
        return (INT_PTR)FALSE;
    }


    static INT_PTR CALLBACK create_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
       // Room* This = (Room*)lParam;
        switch (message)
        {
        case WM_INITDIALOG:
            SetFocus(GetDlgItem(hDlg, IDC_EDIT_ROOMNAME));
            //SendDlgItemMessage(hDlg, IDC_EDIT_ROOMNAME, WM_SETFOCUS, 0, 0);
            return (INT_PTR)false;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                This->create_room(hDlg);
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
        }
        return (INT_PTR)FALSE;
    }
};





bool play_again() {
    return MessageBox(NULL, L"Connect to the same IP?", L"Play again", MB_YESNO) == IDYES;
}

bool set_pos(int &pos) {
    pos = 2;
    if (is_a_server()) {
        if (MessageBox(NULL, L"Please set your position.\n Press Yes to be left or No to be right.", L"Play again", MB_YESNO) == IDYES) {
            pos = 1;
        }
        printf("send:%d\n", pos);
        send_wrap((char*)&pos, sizeof(int));
        recv_wrap((char*)&pos, sizeof(int));
    }
    else {
        recv_wrap((char*)&pos, sizeof(int));
        send_wrap((char*)&pos, sizeof(int));
        pos = (pos == 2) ? 1 : 2;
        printf("recv:%d\n", pos);
    }
    return true;
}


void close_socket(SOCKET* socket) {
    if (*socket == INVALID_SOCKET)
        return;

    shutdown(*socket, SD_SEND);
    closesocket(*socket);
    //WSACleanup();
    *socket = INVALID_SOCKET;
}


INT_PTR CALLBACK interact_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    Room* room = This;
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        HWND hwndOwner;
        RECT rc, rcDlg, rcOwner;

        hwndOwner = GetDesktopWindow();
        GetWindowRect(hwndOwner, &rcOwner);
        GetWindowRect(hDlg, &rcDlg);
        CopyRect(&rc, &rcOwner);

        OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
        OffsetRect(&rc, -rc.left, -rc.top);
        OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);
        SetWindowPos(hDlg, HWND_TOP,
            rcOwner.left + (rc.right / 2),
            rcOwner.top + (rc.bottom / 2),
            0, 0,
            SWP_NOSIZE);


        return (INT_PTR)TRUE;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_CLOSE_SOC:
        {
            close_socket(Connectsocket);
            close_socket(&Connectroomsocket);
            MessageBox(NULL, L"close success", L"Stop Connect", 0);
            return (INT_PTR)TRUE;
        }
        case  IDC_BUTTON_SEARCH:
        {
            room->search_dialog(hDlg);

            return (INT_PTR)TRUE;
        }

        case IDC_BUTTON_CREATE:
        {
            printf("connect socket %d , invalid = %d\n", *Connectsocket, INVALID_SOCKET);
            if (*Connectsocket != INVALID_SOCKET) {
                MessageBox(NULL, L"Close your room first", L"OMG", 0);
                return true;
            }
            room->create_dialog(hDlg);

            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON_CONNECTIP:
        {
            connect_to_server();

            return (INT_PTR)TRUE;
        }
        break;
        }
        break;
    }
    
    case WM_NOTIFY: 
    {
        //wprintf(L"Notify\n");
        switch (((LPNMHDR)lParam)->code)
        {
            //  break;
        case NM_RETURN:
        {
            wprintf(L"Return\n");
            break;
        }
        case NM_CLICK:
        {


            
            HWND g_hLink = GetDlgItem(hDlg, IDC_SYSLINK3);
            PNMLINK pNMLink = (PNMLINK)lParam;
            LITEM   item = pNMLink->item;
            wprintf(L"Click, item->ilink = %d\n", item.iLink);
            if ((((LPNMHDR)lParam)->hwndFrom == g_hLink) )
            {
                wprintf(L"URL: %s\n", item.szUrl);
                ShellExecute(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
            }

            else if (wcscmp(item.szID, L"idInfo") == 0)
            {
                MessageBox(hDlg, L"This isn't much help.", L"Example", MB_OK);
            }

            break;
        }
        }
        return false;
    }
    
    }
    return (INT_PTR)FALSE;
}

void interact_diag() {
    
    //wchar_t c[0x30] = { 0 };
    //wsprintf(c, L"OMG- HWND= %d", hWnd);
    
    //MessageBox(NULL, c, L"???", 0);


    Room room_infos;

    This = &room_infos;
    DialogBoxParam(hMod, MAKEINTRESOURCE(IDD_DIALOG2), NULL, interact_proc,(LPARAM)&room_infos);


}

bool connect_nonblock(SOCKET& nFd, struct addrinfo* ptr);

DWORD WINAPI Room::search_room_socket(LPVOID params) {
    /*
    non-blocking (at connect) version 
    */

    WaitForSingleObject(
        This->ipportstrMutex,    // handle to mutex
        300);

    Connect_infos* room = (Connect_infos*)params;
    char* host = room->ip, * port = room->port;

    printf("IP:%s, port:%s\n", host, port);
    SOCKET Connectroomsocket;
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
    ReleaseMutex(This->ipportstrMutex);
   // delete room;

    if (iresult != 0) {
        printf("False , getaddrifo:%d\n", iresult);
        WSACleanup();
        return 0;
    }
    //printf("getaddrifo:%d\n", iresult);
    ptr = result;
    Connectroomsocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (Connectroomsocket == INVALID_SOCKET) {

        freeaddrinfo(result);
        WSACleanup();
        return 0;
    }
    //SOCKET Connectsocket = INVALID_SOCKET;


    for (ptr = result;ptr != NULL;ptr = ptr->ai_next) {
        Connectroomsocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (Connectroomsocket == INVALID_SOCKET) {
            printf("Create socket failed.\n");
            WSACleanup();
            return 0;
        }

        // set non-blocking socket
        unsigned long nNonBlocking = 1;
        if (ioctlsocket(Connectroomsocket, FIONBIO, &nNonBlocking) == SOCKET_ERROR)
        {
            printf("Unable to set nonblocking mode, ec:%d\n", WSAGetLastError());
            closesocket(Connectroomsocket);
            return false;
        }


        if (connect_nonblock(Connectroomsocket, ptr)) {
            break;
        }
        Connectroomsocket = INVALID_SOCKET;
        /*
        iresult = connect(Connectroomsocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iresult == SOCKET_ERROR) {
            //printf("Connect socket failed.\n");
            closesocket(Connectroomsocket);
            Connectroomsocket = INVALID_SOCKET;
            continue;
        }
        break;
        */

    }

    freeaddrinfo(result);
    if (Connectroomsocket == INVALID_SOCKET) {
       // printf("Unable to connect to server! code-%d\n", WSAGetLastError());
        WSACleanup();
        return 0;
    }
    unsigned long nmode= 0;
    if (ioctlsocket(Connectroomsocket, FIONBIO, &nmode) == SOCKET_ERROR)
    {
        printf("Unable to set back to blocking mode, ec:%d\n", WSAGetLastError());
        closesocket(Connectroomsocket);
        return false;
    }  

    recv(Connectroomsocket, (char*)room->name, sizeof(wchar_t)*0x20,0);
    recv(Connectroomsocket, (char*)room->port, 0x8,0);
    memcpy(room->ip, host, strlen(host)+1);
    
    This->rooms.push_back(*room);
    printf("find ip:%s,port:%s, find_size = %d ", room->ip, room->port, This->rooms.size());
    wprintf(L"name = %s\n", room->name);
    delete room;
    shutdown(Connectroomsocket, SD_SEND);
    closesocket(Connectroomsocket);
    WSACleanup();
    return 1;


}

bool Room::create_room_socket(const char* host,const char* port) {
    SOCKET Connectroomsocket;
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

    if (iresult != 0) {
        printf("False , getaddrifo:%d\n", iresult);
        WSACleanup();
        return 0;
    }
    printf("getaddrifo:%d\n", iresult);
    ptr = result;
    Connectroomsocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (Connectroomsocket == INVALID_SOCKET) {

        freeaddrinfo(result);
        WSACleanup();
        return 0;
    }
    //SOCKET Connectsocket = INVALID_SOCKET;

    iresult = bind(Connectroomsocket, result->ai_addr, (int)result->ai_addrlen);
    if (iresult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(Connectroomsocket);
        WSACleanup();
        return 0;
    }
    freeaddrinfo(result);
    if (listen(Connectroomsocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(Connectroomsocket);
        WSACleanup();
        return 0;
    }

    SOCKET shortsoc;
    while (1) {
        if (!listening)
            break;
        shortsoc = accept(Connectroomsocket, NULL, NULL);
        if (shortsoc == INVALID_SOCKET) {
            printf("room accept failed: %d\n", WSAGetLastError());
            closesocket(shortsoc);
            //WSACleanup();
            return 0;
        }
        printf("accept\n");
        send(shortsoc, (char*)myself.name, sizeof(wchar_t)*0x20,0);
        send(shortsoc, myself.port, 0x8,0);
        printf("find ip:%s,port:%s", myself.ip, myself.port );
        wprintf(L"name = %s\n", myself.name);

        shutdown(shortsoc, SD_SEND);
        closesocket(shortsoc);
        //WSACleanup();
    }
    closesocket(Connectroomsocket);
    printf("get client\n");
    //Connectsocket = tmpsoc;

    return 1;
}


bool connect_nonblock(SOCKET &nFd, struct addrinfo* ptr) {
    // nonblocking connect
    struct timeval oTV;
    oTV.tv_sec = 0;
    oTV.tv_usec = 200*1000;
    fd_set oRead, oWrite;
    FD_ZERO(&oRead);
    FD_ZERO(&oWrite);
    int nResult, nError;
    nResult = connect(nFd, ptr->ai_addr , ptr->ai_addrlen);

    if (nResult == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            printf("Connection failed, ec:%d\n", WSAGetLastError());
            closesocket(nFd);
            return false;
        }
        else // need select
        {
            FD_SET(nFd, &oRead);
            oWrite = oRead;
            nResult = select(nFd + 1, &oRead, &oWrite, 0, &oTV);
            if (nResult == 0)
            {
               // printf("Connection timeout\n");
                closesocket(nFd);
                return false;
            }
            if (FD_ISSET(nFd, &oRead) || FD_ISSET(nFd, &oWrite))
            {
                int nLen = sizeof(nError);
                if (getsockopt(nFd, SOL_SOCKET, SO_ERROR, (char*)&nError, &nLen) < 0)
                {
                    printf("Connect error %d\n", nError);
                    closesocket(nFd);
                    return false;
                }
            }
            else
            {
                printf("Unknown err in connect\n");
                closesocket(nFd);
                return false;
            }
        }
    } // else connected immediately
    return true;
}
