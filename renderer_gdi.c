typedef struct
{
  HWND TargetWindow;
  HDC MemoryDC;
  BITMAPINFO Bitmap;
  uint32_t* Pixels;
} gdi_renderer;

static void
Renderer_Resize(gdi_renderer* Renderer)
{
  RECT WindowRect = {0};
  GetClientRect(Renderer->TargetWindow, &WindowRect);
  int NewWidth = WindowRect.right - WindowRect.left;
  int NewHeight = WindowRect.bottom - WindowRect.top;
  Renderer->Bitmap.bmiHeader.biWidth = NewWidth;
  Renderer->Bitmap.bmiHeader.biHeight = NewHeight; // negative for top-down bitmap
  int AllocSize = sizeof(*Renderer->Pixels) * NewWidth * NewHeight;
  Renderer->Pixels = VirtualAlloc(Renderer->Pixels, AllocSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  ASSERT(Renderer->Pixels);
}

static gdi_renderer*
Renderer_Create(HWND Window)
{
  gdi_renderer* Result = VirtualAlloc(NULL, sizeof(*Result), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  ASSERT(Result);
  Result->TargetWindow = Window;
  Renderer_Resize(Result);
  HDC WindowDC = GetDC(Window);
  Result->MemoryDC = CreateCompatibleDC(WindowDC);
  ReleaseDC(Window, WindowDC);
  Result->Bitmap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  Result->Bitmap.bmiHeader.biPlanes = 1;
  Result->Bitmap.bmiHeader.biBitCount = 32;
  Result->Bitmap.bmiHeader.biCompression = BI_RGB;
  return Result;
}

static void
Renderer_Update(gdi_renderer* Renderer)
{
  int RendererWidth = Renderer->Bitmap.bmiHeader.biWidth;
  int RendererHeight = Renderer->Bitmap.bmiHeader.biHeight;
  HDC WindowDC = GetDC(Renderer->TargetWindow);
  StretchDIBits(WindowDC, 0, 0, RendererWidth, RendererHeight, 0, 0, RendererWidth, RendererHeight, Renderer->Pixels, &Renderer->Bitmap, DIB_RGB_COLORS, SRCCOPY);
  ReleaseDC(Renderer->TargetWindow, WindowDC);
}

static void
Renderer_Clear(gdi_renderer* Renderer, uint32_t ClearColor)
{
  int RendererWidth = Renderer->Bitmap.bmiHeader.biWidth;
  int RendererHeight = Renderer->Bitmap.bmiHeader.biHeight;
  int BufferSize = RendererWidth * RendererHeight;
  for (int Index = 0; Index < BufferSize; Index += 1)
  {
    Renderer->Pixels[Index] = ClearColor;
  }
}

static void
Renderer_DrawPixel(gdi_renderer* Renderer, uint32_t X, uint32_t Y, uint32_t Color)
{
  uint32_t RendererWidth = Renderer->Bitmap.bmiHeader.biWidth;
  uint32_t RendererHeight = Renderer->Bitmap.bmiHeader.biHeight;
  if ((X < RendererWidth) && (Y < RendererHeight))
  {
    Renderer->Pixels[X + (Y * RendererWidth)] = Color;
  }
}

static void
Renderer_Delete(gdi_renderer* Renderer)
{
  VirtualFree(Renderer->Pixels, 0, MEM_RELEASE);
  DeleteDC(Renderer->MemoryDC);
}

static void
Renderer_DrawLine(gdi_renderer* Renderer, int X0, int Y0, int X1, int Y1)
{
  int DeltaX =  ABS(X1 - X0);
  int DeltaY = -ABS(Y1 - Y0);
  int StepX = (X0 < X1) ? 1 : -1;
  int StepY = (Y0 < Y1) ? 1 : -1;
  int Error = DeltaX + DeltaY;
  for (;;)
  {
    Renderer_DrawPixel(Renderer, X0, Y0, 0x00ff0000);
    if (X0 == X1 && Y0 == Y1)
    {
      break;
    }
    int Error2 = Error * 2;
    if (DeltaY <= Error2)
    {
      Error += DeltaY;
      X0 += StepX;
    }
    if (DeltaX >= Error2)
    {
      Error += DeltaX;
      Y0 += StepY;
    }
  }
}