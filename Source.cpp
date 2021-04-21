#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#define WM_USER_SHELLICON (WM_USER + 2)

static bool isWindowCreated = false; //флаг, определяющий создано окно с прицелом или нет

VOID OnImage(HDC); //прототип функции для рисования

static TCHAR szWindowClass[] = _T("DesktopApp_Crosshair"); //название класса "главного" окна

HINSTANCE hInst; //идентификатор приложения

static HWND hWnd_global; //дескриптор окна с прицелом

//прототип функции обработки сообщений от окна
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//главная функция, точка входа
int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    GdiplusStartupInput gdi_in; //переменная структуры, необходима для регистрации
    ULONG_PTR gdi_token; //указатель, также необходим для регистрации

    GdiplusStartup(&gdi_token, &gdi_in, NULL); //инициализация gdi+

    //заполняем структуру класса окна
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

    //регистрация класса окна
    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Не удалось зарегистрировать класс окна!"),
            _T("Ошибка регистрации класса"),
            NULL);

        return 1;
    }

    hInst = hInstance;

    //создаем пустое окно, оно нужно только для задания горячих клавиш
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

    //проверка: если окно создано дескриптор окна содержит его адрес, иначе содержит ноль
    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Не удалось создать окно!"),
            _T("Ошибка создания окна"),
            NULL);

        return 1;
    }

    //задаем hot key для пустого окна
    if (!RegisterHotKey(hWnd, 17, NULL, VK_NUMPAD3))
    {
        MessageBoxW(NULL, _T("Registration Hot Key Wrong (crosshair)"), _T("Hot Key Error"), MB_OK);
    }

    //задание параметров иконки, которая будет находиться в области уведомлений
    NOTIFYICONDATA data{};
    data.cbSize = sizeof(data);
    data.hWnd = hWnd;
    data.uID = 1;
    data.uFlags = NIF_MESSAGE | NIF_ICON;
    data.uCallbackMessage = WM_USER_SHELLICON;
    data.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    data.uVersion = NOTIFYICON_VERSION;

    //добавление иконки в трей
    Shell_NotifyIcon(NIM_ADD, &data);

    //цикл обработки сообщений потока
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //удаление иконки из трея
    Shell_NotifyIcon(NIM_DELETE, &data);

    //освобождаем память, выделенную под gdi+
    GdiplusShutdown(gdi_token);
    return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps; //структура, необходимая для рисования в клиентской области окна
    HDC hdc;

    switch (message) //разбираемся какое именно сообщение пришло
    {
    case WM_HOTKEY: //нажата горячая клавиша
        if (isWindowCreated == false) //флаг - если окно не создано
        {
            int X_max = GetSystemMetrics(SM_CXVIRTUALSCREEN); //x координата правого угла виртуального экрана
            int Y_max = GetSystemMetrics(SM_CYVIRTUALSCREEN); //y координата правого угла виртуального экрана
            int X_min = GetSystemMetrics(SM_XVIRTUALSCREEN); //x координата левого угла виртуального экрана
            int Y_min = GetSystemMetrics(SM_YVIRTUALSCREEN); //y координата левого кгла виртуального экрана

            hWnd_global = NULL;

            //создаем окно
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

            //проверка, получилось ли создать окно
            if (!hWnd_global)
            {
                MessageBox(NULL,
                    _T("Не удалось создать окно для прицела!"),
                    _T("Ошибка создания окна"),
                    NULL);

                return 1;
            }


            HRGN hrgn_hot = CreateRectRgn(X_max / 2 - 215, Y_max / 2 - 205, X_max / 2 + 230, Y_max / 2 + 240); //задаем переменную для региона в окне
            SetWindowRgn(hWnd_global, hrgn_hot, TRUE); //устанавливаем регион

            SetLayeredWindowAttributes(hWnd_global, RGB(255, 255, 255), 255, LWA_COLORKEY); //задаем цвет для прозрачности

            ShowWindow(hWnd_global, 1); //показываем окно
            UpdateWindow(hWnd_global); //обновляем окно(рисуем прицел)

            isWindowCreated = true; //меняем значение флага
        }
        else if (isWindowCreated == true) //если окно создано
        {
            DestroyWindow(hWnd_global); //уничтожаем окно
        }
        break;
    case WM_PAINT: //надо нарисовать изображение
        hdc = BeginPaint(hWnd, &ps);

        OnImage(hdc); //рисуем картинку прицела

        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY: //уничтожение окна
        isWindowCreated = false;
        break;
    case WM_USER_SHELLICON: //нажатие на иконку в трее левой или правой кнопкой мыши
        if (lParam == WM_RBUTTONDOWN || lParam == WM_LBUTTONDOWN)
            if (MessageBoxW(NULL, _T("Завершить работу?"), _T("Прицел"), MB_YESNO | MB_TOPMOST) == IDYES)
                PostQuitMessage(0);
    default: //если сообщение не совпадает ни с одним из выше указанных
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}


VOID OnImage(HDC hdc2) //определение функции для рисования
{
    Graphics graphics(hdc2); //инициализация объекта graphics

    Image image(_T("Custom.png")); //инициализация объекта image

    graphics.DrawImage(&image, GetSystemMetrics(SM_CXVIRTUALSCREEN) / 2 - 33, GetSystemMetrics(SM_CYVIRTUALSCREEN) / 2 - 56); //функция рисования изображения с помощью gdi+
}
