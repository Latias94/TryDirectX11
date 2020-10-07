#include "App.h"

// Win32 程序必须使用 CALLBACK 的方式，因为 WinApi 使用 stdcall，
// 使用 WinApi 创建的函数也要使用 stdcall
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    try {
        return App{}.Go();
    }
    catch (const ChiliException &e) {
        MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
    }
    catch (const std::exception &e) {
        MessageBoxA(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
    }
    catch (...) {
        MessageBox(nullptr, _T("No details available"), _T("Unknown Exception"), MB_OK | MB_ICONEXCLAMATION);
    }
    return -1;
}
