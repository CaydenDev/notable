#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <fstream>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")  

#define ID_FILE_NEW 1
#define ID_FILE_OPEN 2
#define ID_FILE_SAVE 3
#define ID_FILE_SAVEAS 4
#define ID_FILE_EXIT 5
#define ID_EDIT_UNDO 6
#define ID_EDIT_REDO 7
#define ID_EDIT_COPY 8
#define ID_EDIT_PASTE 9
#define ID_VIEW_DARKMODE 10

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND);
void AddControls(HWND);
void OpenFile(HWND);
void SaveFile(HWND, BOOL);
void NewFile(HWND);
void SetDarkMode(HWND, bool);

HMENU hMenu;
HWND hEdit;
bool darkMode = false;
wchar_t fileName[MAX_PATH] = L"";
HWND hTab;


int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR args, int ncmdshow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASS wc = { 0 };
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"NoteTakingApp";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClass(&wc)) {
        return -1;
    }

    HWND hWnd = CreateWindowW(
        wc.lpszClassName, 
        L"Note Taking Application",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1000,
        800,
        NULL,
        NULL,
        hInst,
        NULL
    );

    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND:
        switch (wp) {
        case ID_FILE_NEW:
            NewFile(hWnd);
            break;
        case ID_FILE_OPEN:
            OpenFile(hWnd);
            break;
        case ID_FILE_SAVE:
            SaveFile(hWnd, FALSE);
            break;
        case ID_FILE_SAVEAS:
            SaveFile(hWnd, TRUE);
            break;
        case ID_FILE_EXIT:
            PostQuitMessage(0);
            break;
        case ID_VIEW_DARKMODE:
            darkMode = !darkMode;
            SetDarkMode(hWnd, darkMode);
            break;
        }
        break;
    case WM_CREATE:
        AddMenus(hWnd);
        AddControls(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wp, lp);
    }
    return 0;
}

void AddMenus(HWND hWnd) {
    hMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    HMENU hEditMenu = CreateMenu();
    HMENU hViewMenu = CreateMenu();

    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_NEW, L"New");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_OPEN, L"Open");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_SAVE, L"Save");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_SAVEAS, L"Save As");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_EXIT, L"Exit");

    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_DARKMODE, L"Toggle Dark Mode");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, L"Edit");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, L"View");

    SetMenu(hWnd, hMenu);
}

void AddControls(HWND hWnd) {
    
    hTab = CreateWindowW(WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE | TCS_TABS, 0, 0, 1000, 30, hWnd, NULL, NULL, NULL);

    
    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPWSTR)L"Note 1"; 

    SendMessage(hTab, TCM_INSERTITEM, 0, (LPARAM)&tie);

    
    hEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT", 
        L"", 
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        10,
        40,
        960,
        720,
        hWnd,
        NULL,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL
    );
}

void OpenFile(HWND hWnd) {
    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetOpenFileName(&ofn)) {
        std::ifstream file(ofn.lpstrFile);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            SetWindowTextA(hEdit, content.c_str());
            file.close();
            wcscpy_s(fileName, MAX_PATH, ofn.lpstrFile);
        }
    }
}

void SaveFile(HWND hWnd, BOOL saveAs) {
    if (saveAs || wcscmp(fileName, L"") == 0) {
        OPENFILENAME ofn;
        wchar_t szFile[260] = { 0 };

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
        ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

        if (GetSaveFileName(&ofn)) {
            std::ofstream file(ofn.lpstrFile);
            if (file.is_open()) {
                char buffer[1024];
                GetWindowTextA(hEdit, buffer, sizeof(buffer));
                file << buffer;
                file.close();
                wcscpy_s(fileName, MAX_PATH, ofn.lpstrFile);
            }
        }
    }
    else {
        std::ofstream file(fileName);
        if (file.is_open()) {
            char buffer[1024];
            GetWindowTextA(hEdit, buffer, sizeof(buffer));
            file << buffer;
            file.close();
        }
    }
}

void NewFile(HWND hWnd) {
    SetWindowTextA(hEdit, "");
    wcscpy_s(fileName, MAX_PATH, L"");
}

void SetDarkMode(HWND hwnd, bool enable) {
    if (enable) {
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(40, 40, 40)));
        InvalidateRect(hwnd, NULL, TRUE);
    }
    else {
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)(COLOR_WINDOW + 1));
        InvalidateRect(hwnd, NULL, TRUE);
    }
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
}
