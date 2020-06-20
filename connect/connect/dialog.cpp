
#include "pch.h"
#include <comdef.h>
#include "dialog.h"

wchar_t ip_port[2][0x30] = { {0}, { 0 } };
wchar_t* buf[2] = { ip_port[0],ip_port[1] };

#define BUF_SIZE 0x40


INT_PTR CALLBACK proc_port(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK proc_host(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


DWORD pid;
BOOL CALLBACK on_window_enum(HWND hwnd, LPARAM lparam) {
    DWORD lpdwProcessId;
    /*
    printf("pid = %d\n", GetWindowThreadProcessId(hwnd, &lpdwProcessId));
    printf("lpdwProcessId = %d\n", lpdwProcessId);
    */
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);
    if (pid == lpdwProcessId) {
        *(reinterpret_cast<HWND*>(lparam)) = hwnd;
        printf("\nget\n");
        return false;
    }
    return true;
};

void get_hwnd_current_proc(HWND &hwndfind) {
    pid = GetCurrentProcessId();
    LPDWORD lpdwProcessId;
    //HWND hwndfind = NULL;

    EnumWindows(on_window_enum, reinterpret_cast<LPARAM>(&hwndfind));
    
}

wchar_t** get_host() {

    // refer to https://stackoverflow.com/questions/24099903/c-enumwindows-and-then-perform-checks
    // https://stackoverflow.com/questions/11711417/get-hwnd-by-process-id-c
    wchar_t title[BUF_SIZE];
    GetConsoleTitle(title, BUF_SIZE);
    HWND hwndfind = NULL;

    wprintf(L"title:%s", title);
    get_hwnd_current_proc(hwndfind);
    //hwndfind = GetCurrentProcess();
    //HWND hwndfind = get_hwnd_current_proc();//FindWindow(NULL, title);
    int msgboxID = 2;


    if (DialogBox(hMod, MAKEINTRESOURCE(IDD_DIALOG), hwndfind, proc_host) == IDOK) {
        //_bstr_t  c(buf);
        
        return (wchar_t**)buf;
    }
    else {
        return 0;
    }
}

wchar_t** get_port() {
    wchar_t title[BUF_SIZE];
    GetConsoleTitle(title, BUF_SIZE);
    HWND hwndfind = NULL;

    wprintf(L"title:%s", title);
    get_hwnd_current_proc(hwndfind);
    int msgboxID = 2;


    if (DialogBox(hMod, MAKEINTRESOURCE(IDD_DIALOG1), hwndfind, proc_port) == IDOK) {
        //_bstr_t  c(buf);

        return (wchar_t**)buf;
    }
    else {
        return 0;
    }
}



INT_PTR CALLBACK proc_host(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            
            HWND getitem = GetDlgItem(hDlg, IDC_EDIT1);
            //SendMessage(getitem, WM_GETTEXT, 255, (LPARAM)buf);
            GetDlgItemText(hDlg, IDC_EDIT1, buf[0], 64);
            GetDlgItemText(hDlg, IDC_EDIT2, buf[1], 64);
            printf("port_len:%d", lstrlenW(buf[1]));
            //if(lstrlenW(buf[1]))
            wchar_t cntstr[0x80] = { 0 };
            wsprintf(cntstr, L"%s:%s", buf[0], buf[1]);
            MessageBox(hDlg, cntstr, L"Connect to", 0);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK proc_port(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {

            HWND getitem = GetDlgItem(hDlg, IDC_EDIT1);
            GetDlgItemText(hDlg, IDC_EDIT3, buf[1], 64);
            printf("port_len:%d", lstrlenW(buf[1]));
            wchar_t cntstr[0x80] = { 0 };
            wsprintf(cntstr, L"%s:%s", L"localhost", buf[1]);
            MessageBox(hDlg, cntstr, L"Bind", 0);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
