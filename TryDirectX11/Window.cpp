/******************************************************************************************
*	Chili Direct3D Engine																  *
*	Copyright 2018 PlanetChili <http://www.planetchili.net>								  *
*																						  *
*	This file is part of Chili Direct3D Engine.											  *
*																						  *
*	Chili Direct3D Engine is free software: you can redistribute it and/or modify		  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The Chili Direct3D Engine is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The Chili Direct3D Engine.  If not, see <http://www.gnu.org/licenses/>.    *
******************************************************************************************/
#include "Window.h"
#include <sstream>
#include "resource.h"
#include "WindowsThrowMacros.h"

// Window Class Stuff
Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept
        :
        hInst(GetModuleHandle(nullptr)) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetInstance();
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = GetName();
    wc.hIconSm = nullptr;
    RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass() {
    UnregisterClass(wndClassName, GetInstance());
}

LPCTSTR Window::WindowClass::GetName() noexcept {
    return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept {
    return wndClass.hInst;
}


// Window Stuff
Window::Window(int width, int height, LPCTSTR name) : width(width), height(height) {
    // calculate window size based on desired client region size
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;
    if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0) {
        throw CHWND_LAST_EXCEPT();
    }
    // create window & get hWnd
    hWnd = CreateWindow(
            WindowClass::GetName(), name,
            WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
            nullptr, nullptr, WindowClass::GetInstance(), this
    );

    if (hWnd == nullptr) {
        throw CHWND_LAST_EXCEPT();
    }
    // newly create windows start off as hidden
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    // create graphics object
    // unique 指针，在窗口销毁的时候也会自动销毁这个对象
    pGfx = std::make_unique<Graphics>(hWnd);
}

Window::~Window() {
    DestroyWindow(hWnd);
}

void Window::SetTitle(const std::string &title) {
    if (SetWindowTextA(hWnd, title.c_str()) == 0) {
        throw CHWND_LAST_EXCEPT();
    }
}

std::optional<int> Window::ProcessMessages()
{
    MSG msg;
    // while queue has messages, remove and dispatch them (but do not block on empty queue)
    while( PeekMessage( &msg,nullptr,0,0,PM_REMOVE ) )
    {
        // check for quit because peekmessage does not signal this via return val
        if( msg.message == WM_QUIT )
        {
            // return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
            return (int)msg.wParam;
        }

        // TranslateMessage will post auxilliary WM_CHAR messages from key msgs
        // TranslateMessage 函数实现了键盘按键的转换，特别是将虚拟键 WM_KEYDOWN 消息
        // 转换为字符消息 WM_CHAR
        TranslateMessage(&msg);
        // DispatchMessage 函数则会把消息分派给相应的窗口过程。
        DispatchMessage(&msg);
//            if (wnd.kbd.KeyIsPressed(VK_SPACE)) {
//                MessageBox(nullptr, _T("Something Happon!"), _T("Space Key"), MB_OK);
//            }
//            while (!wnd.mouse.IsEmpty()){
//                const auto e = wnd.mouse.Read();
//                if(e.GetType()==Mouse::Event::Type::Move){
//                    std::ostringstream oss;
//                    oss << "Mouse Position: (" << e.GetPosX() << "," << e.GetPosY() << ")" << std::endl;
//                    wnd.SetTitle(oss.str());
//                }
//            }
    }

    // return empty optional when not quitting app
    return {};
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    // use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
    if (msg == WM_NCCREATE) {
        // extract ptr to window class from creation data
        const CREATESTRUCTW *const pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
        Window *const pWnd = static_cast<Window *>(pCreate->lpCreateParams);
        // set WinAPI-managed user data to store ptr to window class
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        // set message proc to normal (non-setup) handler now that setup is finished
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
        // forward message to window class handler
        return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
    }
    // if we get a message before the WM_NCCREATE message, handle with default handler
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// 不能通过 WINAPI来调用成员函数，因此创建静态函数
LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    // retrieve ptr to window class
    Window *const pWnd = reinterpret_cast<Window *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    // forward message to window class handler
    return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    switch (msg) {
        // we don't want the DefProc to handle this message because
        // we want our destructor to destroy the window, so return 0 instead of break
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
            // clear keystate when window loses focus to prevent input getting "stuck"
            // 焦点不在当前窗口下时，清除键盘的所有状态
        case WM_KILLFOCUS:
            kbd.ClearState();
            break;

            /*********** KEYBOARD MESSAGES ***********/
        case WM_KEYDOWN:
            // syskey commands need to be handled to track ALT key (VK_MENU) and F10
        case WM_SYSKEYDOWN:
            if (!(lParam & 0x40000000) || kbd.AutorepeatIsEnabled()) // filter autorepeat
            {
                kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
            }
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
            break;
        case WM_CHAR:
            kbd.OnChar(static_cast<unsigned char>(wParam));
            break;
            /*********** END KEYBOARD MESSAGES ***********/

            /************* MOUSE MESSAGES ****************/
        case WM_MOUSEMOVE: {
            const POINTS pt = MAKEPOINTS(lParam);
            // in client region -> log move, and log enter + capture mouse (if not previously in window)
            if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height) {
                mouse.OnMouseMove(pt.x, pt.y);
                if (!mouse.IsInWindow()) {
                    SetCapture(hWnd);
                    mouse.OnMouseEnter();
                }
            }
                // not in client -> log move / maintain capture if button down
            else {
                if (wParam & (MK_LBUTTON | MK_RBUTTON)) {
                    mouse.OnMouseMove(pt.x, pt.y);
                }
                    // button up -> release capture / log event for leaving
                else {
                    ReleaseCapture();
                    mouse.OnMouseLeave();
                }
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            const POINTS pt = MAKEPOINTS(lParam);
            mouse.OnLeftPressed(pt.x, pt.y);
            break;
        }
        case WM_RBUTTONDOWN: {
            const POINTS pt = MAKEPOINTS(lParam);
            mouse.OnRightPressed(pt.x, pt.y);
            break;
        }
        case WM_LBUTTONUP: {
            const POINTS pt = MAKEPOINTS(lParam);
            mouse.OnLeftReleased(pt.x, pt.y);
            // release mouse if outside of window
            if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height) {
                ReleaseCapture();
                mouse.OnMouseLeave();
            }
            break;
        }
        case WM_RBUTTONUP: {
            const POINTS pt = MAKEPOINTS(lParam);
            mouse.OnRightReleased(pt.x, pt.y);
            // release mouse if outside of window
            if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height) {
                ReleaseCapture();
                mouse.OnMouseLeave();
            }
            break;
        }
        case WM_MOUSEWHEEL: {
            const POINTS pt = MAKEPOINTS(lParam);
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            mouse.OnWheelDelta(pt.x, pt.y, delta);
            break;
        }
            /************** END MOUSE MESSAGES **************/
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

Graphics &Window::Gfx() {
    // 获取图形类，但还没设置指针时报错
    if (!pGfx) {
        throw CHWND_NOGFX_EXCEPT();
    }
    return *pGfx;
}

// Window Exception Stuff
Window::HrException::HrException(int line, const char *file, HRESULT hr) noexcept
        :
        Exception(line, file),
        hr(hr) {}

const char *Window::HrException::what() const noexcept {
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[Error Code] " << GetErrorCode() << std::endl
        << "[Description] " << GetErrorDescription() << std::endl
        << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char *Window::HrException::GetType() const noexcept {
    return "Chili Window Exception";
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept {
    char *pMsgBuf = nullptr;
    // windows will allocate memory for err string and make our pointer point to it
    DWORD nMsgLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPTSTR>(&pMsgBuf), 0, nullptr
    );
    // 0 string length returned indicates a failure
    if (nMsgLen == 0) {
        return "Unidentified error code";
    }
    // copy error string from windows-allocated buffer to std::string
    std::string errorString = pMsgBuf;
    // free windows buffer
    LocalFree(pMsgBuf);
    return errorString;
}

HRESULT Window::HrException::GetErrorCode() const noexcept {
    return hr;
}

std::string Window::HrException::GetErrorDescription() const noexcept {
    return Exception::TranslateErrorCode(hr);
}

const char* Window::NoGfxException::GetType() const noexcept
{
    return "Chili Window Exception [No Graphics]";
}