#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#define DEBUG_BREAK() __debugbreak()
#include<windows.h>
#include<stdint.h>

#include"header.h"
#include"renderer_gdi.c"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

static LRESULT WINAPI
WindowProc(HWND Window, UINT Message, WPARAM Wparam, LPARAM Lparam)
{
  LRESULT Result = 0;
  switch (Message)
  {
    case WM_CLOSE:
    {
      PostQuitMessage(0);
    } break;
    case WM_CHAR:
    {
      if (Wparam == VK_ESCAPE)
      {
        PostMessageW(Window, WM_CLOSE, 0, 0);
      }
    } break;
    case WM_SIZE:
    {
      gdi_renderer* Renderer = (gdi_renderer*)GetWindowLongPtrW(Window, GWLP_USERDATA);
      if (Renderer)
      {
        Renderer_Resize(Renderer);
      }
    } break;
    default:
    {
      Result = DefWindowProcW(Window, Message, Wparam, Lparam);
    }
  }
  return Result;
}

static HWND
CreateOutputWindow(void)
{
  WNDCLASSEXW WindowClass = {0};
  WindowClass.cbSize = sizeof(WindowClass);
  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.hInstance = GetModuleHandleW(NULL);
  WindowClass.lpfnWndProc = WindowProc;
  WindowClass.lpszClassName = L"renderer_wndclass";
  RegisterClassExW(&WindowClass);
  DWORD WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  HWND Result = CreateWindowExW(0, WindowClass.lpszClassName, L"Window", WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, WindowClass.hInstance, NULL);
  ASSERT(IsWindow(Result));
  return Result;
}

int WINAPI
wWinMain(HINSTANCE Instance, HINSTANCE Unused, LPWSTR CmdLine, int CmdShow)
{
  int ExitCode = 0;
  HWND Window = CreateOutputWindow();
  gdi_renderer* Renderer = Renderer_Create(Window);
  SetWindowLongPtrW(Window, GWLP_USERDATA, (LONG_PTR)Renderer);

  MSG Message = {0};
  for(;;)
  {
    if(PeekMessageW(&Message, NULL, 0, 0, PM_REMOVE))
    {
      if(Message.message == WM_QUIT)
      {
        break;
      }
      TranslateMessage(&Message);
      DispatchMessageW(&Message);
    }
    Renderer_Clear(Renderer, 0x00446688);
    Renderer_Update(Renderer);
  }
  return ExitCode;
}