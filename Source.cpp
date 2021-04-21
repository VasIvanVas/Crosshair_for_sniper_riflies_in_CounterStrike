#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#define WM_USER_SHELLICON (WM_USER + 2)

static bool isWindowCreated = false; //����, ������������ ������� ���� � �������� ��� ���

VOID OnImage(HDC); //�������� ������� ��� ���������

static TCHAR szWindowClass[] = _T("DesktopApp_Crosshair"); //�������� ������ "��������" ����

HINSTANCE hInst; //������������� ����������

static HWND hWnd_global; //���������� ���� � ��������

//�������� ������� ��������� ��������� �� ����
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//������� �������, ����� �����
int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    GdiplusStartupInput gdi_in; //���������� ���������, ���������� ��� �����������
    ULONG_PTR gdi_token; //���������, ����� ��������� ��� �����������

    GdiplusStartup(&gdi_token, &gdi_in, NULL); //������������� gdi+

    //��������� ��������� ������ ����
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    //����������� ������ ����
    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("�� ������� ���������������� ����� ����!"),
            _T("������ ����������� ������"),
            NULL);

        return 1;
    }

    hInst = hInstance;

    //������� ������ ����, ��� ����� ������ ��� ������� ������� ������
    HWND hWnd = CreateWindowEx(WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        szWindowClass,
        _T("WindowForHotKey"),
        NULL,
        0,
        0,
        0,
        0,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    //��������: ���� ���� ������� ���������� ���� �������� ��� �����, ����� �������� ����
    if (!hWnd)
    {
        MessageBox(NULL,
            _T("�� ������� ������� ����!"),
            _T("������ �������� ����"),
            NULL);

        return 1;
    }

    //������ hot key ��� ������� ����
    if (!RegisterHotKey(hWnd, 17, NULL, VK_NUMPAD3))
    {
        MessageBoxW(NULL, _T("Registration Hot Key Wrong (crosshair)"), _T("Hot Key Error"), MB_OK);
    }

    //������� ���������� ������, ������� ����� ���������� � ������� �����������
    NOTIFYICONDATA data{};
    data.cbSize = sizeof(data);
    data.hWnd = hWnd;
    data.uID = 1;
    data.uFlags = NIF_MESSAGE | NIF_ICON;
    data.uCallbackMessage = WM_USER_SHELLICON;
    data.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    data.uVersion = NOTIFYICON_VERSION;

    //���������� ������ � ����
    Shell_NotifyIcon(NIM_ADD, &data);

    //���� ��������� ��������� ������
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //�������� ������ �� ����
    Shell_NotifyIcon(NIM_DELETE, &data);

    //����������� ������, ���������� ��� gdi+
    GdiplusShutdown(gdi_token);
    return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps; //���������, ����������� ��� ��������� � ���������� ������� ����
    HDC hdc;

    switch (message) //����������� ����� ������ ��������� ������
    {
    case WM_HOTKEY: //������ ������� �������
        if (isWindowCreated == false) //���� - ���� ���� �� �������
        {
            int X_max = GetSystemMetrics(SM_CXVIRTUALSCREEN); //x ���������� ������� ���� ������������ ������
            int Y_max = GetSystemMetrics(SM_CYVIRTUALSCREEN); //y ���������� ������� ���� ������������ ������
            int X_min = GetSystemMetrics(SM_XVIRTUALSCREEN); //x ���������� ������ ���� ������������ ������
            int Y_min = GetSystemMetrics(SM_YVIRTUALSCREEN); //y ���������� ������ ���� ������������ ������

            hWnd_global = NULL;

            //������� ����
            hWnd_global = CreateWindowEx(WS_EX_WINDOWEDGE | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT,
                szWindowClass,
                _T(""),
                WS_OVERLAPPEDWINDOW & ~WS_SYSMENU & ~WS_CAPTION,
                X_min,
                Y_min,
                X_max,
                Y_max,
                NULL,
                NULL,
                hInst,
                NULL
            );

            //��������, ���������� �� ������� ����
            if (!hWnd_global)
            {
                MessageBox(NULL,
                    _T("�� ������� ������� ���� ��� �������!"),
                    _T("������ �������� ����"),
                    NULL);

                return 1;
            }


            HRGN hrgn_hot = CreateRectRgn(X_max / 2 - 215, Y_max / 2 - 205, X_max / 2 + 230, Y_max / 2 + 240); //������ ���������� ��� ������� � ����
            SetWindowRgn(hWnd_global, hrgn_hot, TRUE); //������������� ������

            SetLayeredWindowAttributes(hWnd_global, RGB(255, 255, 255), 255, LWA_COLORKEY); //������ ���� ��� ������������

            ShowWindow(hWnd_global, 1); //���������� ����
            UpdateWindow(hWnd_global); //��������� ����(������ ������)

            isWindowCreated = true; //������ �������� �����
        }
        else if (isWindowCreated == true) //���� ���� �������
        {
            DestroyWindow(hWnd_global); //���������� ����
        }
        break;
    case WM_PAINT: //���� ���������� �����������
        hdc = BeginPaint(hWnd, &ps);

        OnImage(hdc); //������ �������� �������

        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY: //����������� ����
        isWindowCreated = false;
        break;
    case WM_USER_SHELLICON: //������� �� ������ � ���� ����� ��� ������ ������� ����
        if (lParam == WM_RBUTTONDOWN || lParam == WM_LBUTTONDOWN)
            if (MessageBoxW(NULL, _T("��������� ������?"), _T("������"), MB_YESNO | MB_TOPMOST) == IDYES)
                PostQuitMessage(0);
    default: //���� ��������� �� ��������� �� � ����� �� ���� ���������
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}


VOID OnImage(HDC hdc2) //����������� ������� ��� ���������
{
    Graphics graphics(hdc2); //������������� ������� graphics

    Image image(_T("Custom.png")); //������������� ������� image

    graphics.DrawImage(&image, GetSystemMetrics(SM_CXVIRTUALSCREEN) / 2 - 33, GetSystemMetrics(SM_CYVIRTUALSCREEN) / 2 - 56); //������� ��������� ����������� � ������� gdi+
}
