#include <windows.h>
#include <Windowsx.h>

#include <dwrite.h>
#include <iostream>
#pragma comment(lib, "Dwrite")

#include "basewin.h"
#include "Quickhull.h"

class MainWindow : public BaseWindow<MainWindow> {
    wchar_t*           currentAlgLoaded;
    //tools for writing the text in the boxes
    IDWriteFactory* pWriteFactory;
    ID2D1SolidColorBrush* pTextBrush;
    IDWriteTextFormat* pTextFormat;
    //tools for drawing everything
    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrush;
    ID2D1SolidColorBrush* pBrushYellow;
    ID2D1SolidColorBrush* pBrushWhite;
    //the text rectangles on the left
    D2D1_RECT_F           rect1;//Minkowski Difference
    D2D1_RECT_F           rect2;//Minkowski Sum
    D2D1_RECT_F           rect3;//Quickhull
    D2D1_RECT_F           rect4;//Point Convex Hull
    D2D1_RECT_F           rect5;//GJK
    //the points for the line outline of the program
    D2D1_POINT_2F         topLeft;
    D2D1_POINT_2F         topRight;
    D2D1_POINT_2F         bottomLeft;
    D2D1_POINT_2F         bottomRight;
    D2D1_POINT_2F         listLeftTop;
    D2D1_POINT_2F         listLeftBottom;

    BOOL    IsInRect(int mouseX, int mouseY, D2D1_RECT_F rect);
    void    OnLButtonDown(int pixelX, int pixelY);

    void    CalculateLayout();
    HRESULT CreateTextResources();
    HRESULT CreateGraphicsResources();
    void    DiscardTextResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();

public:

    MainWindow() : pFactory(nullptr), pRenderTarget(nullptr), pBrush(nullptr) {}

    PCWSTR  ClassName() const { return L"Convex Hull Algorithms Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

class DPIScale{
    static float DPIscaleX;
    static float DPIscaleY;

public:
    static void Initialize(ID2D1Factory* pFactory){
        FLOAT dpiX, dpiY;
        pFactory->GetDesktopDpi(&dpiX, &dpiY);
        DPIscaleX = dpiX / 96.0f;
        DPIscaleY = dpiY / 96.0f;
    }
    template <typename T>static float PixelsToDipsX(T x){
        return static_cast<float>(x) / DPIscaleX;
    }
    template <typename T>static float PixelsToDipsY(T y){
        return static_cast<float>(y) / DPIscaleY;
    }
};
float DPIScale::DPIscaleX = 1.0f;
float DPIScale::DPIscaleY = 1.0f;


//Calculates that new layout of the buttons on the left based on the screen size
void MainWindow::CalculateLayout() {
    if (pRenderTarget != nullptr) {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        const float farthestX = size.width / 3;
        const float xSpace = (size.width / 3) * .025;
        const float smallestX = xSpace;
        const float largestX = farthestX - xSpace;

        const float ySize = size.height * .1;
        const float ySpace = size.height * .05;
        const float y1 = ySpace;
        const float y2 = y1 + ySpace + ySize;
        const float y3 = y2 + ySpace + ySize;
        const float y4 = y3 + ySpace + ySize;
        const float y5 = y4 + ySpace + ySize;

        topLeft = D2D1::Point2F(0,0);
        topRight = D2D1::Point2F(size.width,0);
        bottomLeft = D2D1::Point2F(0,size.height);
        bottomRight = D2D1::Point2F(size.width, size.height);

        listLeftTop = D2D1::Point2F(farthestX, 0);
        listLeftBottom = D2D1::Point2F(farthestX, size.height);


        rect1 = D2D1::RectF(smallestX, y1, largestX, y1 + ySize);
        rect2 = D2D1::RectF(smallestX, y2, largestX, y2 + ySize);
        rect3 = D2D1::RectF(smallestX, y3, largestX, y3 + ySize);
        rect4 = D2D1::RectF(smallestX, y4, largestX, y4 + ySize);
        rect5 = D2D1::RectF(smallestX, y5, largestX, y5 + ySize);
    }
}

//handels the initialization of the pRenderTarget
HRESULT MainWindow::CreateGraphicsResources() {
    HRESULT hr = S_OK;
    HRESULT hr2 = S_OK;
    HRESULT hr3 = S_OK;
    HRESULT hr4 = S_OK;
    if (pRenderTarget == nullptr) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
        hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hwnd, size), &pRenderTarget);
        if (SUCCEEDED(hr)) {
            hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &pBrush);
            hr2 = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pTextBrush);
            hr3 = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &pBrushYellow);
            hr4 = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrushWhite);

            if (SUCCEEDED(hr) && SUCCEEDED(hr2) && SUCCEEDED(hr3) && SUCCEEDED(hr4)) {
                CalculateLayout();
            }
        }
    }
    return hr;
}
//creates the text formating(font, font size etc)
HRESULT MainWindow::CreateTextResources() {
    HRESULT hr = S_OK;
    hr = pWriteFactory->CreateTextFormat(
        L"Arial",
        NULL,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        36.0f,
        L"en-us",
        &pTextFormat
    );
    if (SUCCEEDED(hr)) {
        pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    
    return hr;
}

//removes the memory alocation for a given variable
template <class T> void SafeRelease(T** x) {
    if (*x) {
        (*x)->Release();
        *x = nullptr;
    }
}

void MainWindow::DiscardGraphicsResources() {
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::DiscardTextResources() {
    SafeRelease(&pTextBrush);
    SafeRelease(&pTextFormat);
}

//the onpaint method which redraws the rectangles on the left along with the lines surrounding the app
void MainWindow::OnPaint() {
    HRESULT hr = CreateGraphicsResources();
    HRESULT hr2 = CreateTextResources();
    if (SUCCEEDED(hr) ) {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
        pRenderTarget->FillRectangle(rect1, pBrush);
        pRenderTarget->FillRectangle(rect2, pBrush);
        pRenderTarget->FillRectangle(rect3, pBrush);
        pRenderTarget->FillRectangle(rect4, pBrush);
        pRenderTarget->FillRectangle(rect5, pBrush);
        pRenderTarget->DrawLine(listLeftTop, listLeftBottom, pBrush, 4.0f);
        pRenderTarget->DrawLine(topLeft, topRight, pBrush, 4.0f);
        pRenderTarget->DrawLine(topLeft, bottomLeft, pBrush, 4.0f);
        pRenderTarget->DrawLine(bottomLeft, bottomRight, pBrush, 4.0f);
        pRenderTarget->DrawLine(topRight, bottomRight, pBrush, 4.0f);
        pRenderTarget->DrawTextW(L"Minkowski Difference", 20, pTextFormat, rect1, pTextBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP, DWRITE_MEASURING_MODE_NATURAL);
        pRenderTarget->DrawTextW(L"Minkowski Sum", 13, pTextFormat, rect2, pTextBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP, DWRITE_MEASURING_MODE_NATURAL);
        pRenderTarget->DrawTextW(L"Quickhull", 9, pTextFormat, rect3, pTextBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP, DWRITE_MEASURING_MODE_NATURAL);
        pRenderTarget->DrawTextW(L"Point Convex Hull", 17, pTextFormat, rect4, pTextBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP, DWRITE_MEASURING_MODE_NATURAL);
        pRenderTarget->DrawTextW(L"GJK", 3, pTextFormat, rect5, pTextBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP, DWRITE_MEASURING_MODE_NATURAL);
        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
            DiscardGraphicsResources();
            DiscardTextResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

//called when the window size is changed
void MainWindow::Resize() {
    if (pRenderTarget != nullptr) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        CalculateLayout();
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

//checks if this mouse click is in a given rectangle
BOOL MainWindow::IsInRect(int pixelX, int pixelY, D2D1_RECT_F rect) {
    const float dipX = DPIScale::PixelsToDipsX(pixelX);
    const float dipY = DPIScale::PixelsToDipsY(pixelY);
    if (rect.left<=dipX && rect.right >=dipX && rect.top<=dipY && rect.bottom>=dipY) {
        return true;
    }
    return false;
}

//handles the left button clicks
void MainWindow::OnLButtonDown(int pixelX, int pixelY) {
    D2D1_SIZE_F size = pRenderTarget->GetSize();
    if (IsInRect(pixelX, pixelY, rect1)) {
        currentAlgLoaded = L"minDiff";
        OnPaint();
        //Minkowski Difference
    }
    else if (IsInRect(pixelX, pixelY, rect2)) {
        currentAlgLoaded = L"minSum";
        OnPaint();
        //Minkowski sum
    }
    else if (IsInRect(pixelX, pixelY, rect3)) {
        currentAlgLoaded = L"quickhull";
        OnPaint();
        //Quickhull
    }
    else if (IsInRect(pixelX, pixelY, rect4)) {
        currentAlgLoaded = L"pointConvex";
        OnPaint();

        //Point Convex Hull
    }
    else if (IsInRect(pixelX, pixelY, rect5)) {
        currentAlgLoaded = L"gjk";
        OnPaint();
        //Gjk
    }
}

//handles all the messages given to this app from windows
LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))){
            return -1;
        }else {
            DPIScale::Initialize(pFactory);
            if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**> (&pWriteFactory)))) {
                return -1;
            }
        }
        return 0;
    case WM_DESTROY:
        DiscardGraphicsResources();
        DiscardTextResources();
        SafeRelease(&pFactory);
        SafeRelease(&pWriteFactory);
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        OnPaint();
        return 0;
    case WM_SIZE:
        Resize();
        return 0;
    case WM_LBUTTONDOWN:
        OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_LBUTTONUP://may not be necessary
//        OnLButtonUp();
        return 0;
    case WM_MOUSEMOVE:
//        OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        return 0;
    case WM_SETCURSOR://may not be necessary
        if (LOWORD(lParam) == HTCLIENT)
        {
//            SetCursor(hCursor);
            return TRUE;
        }
        break;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    MainWindow win;
    if (!win.Create(L"Convex Hull Algorithms", WS_OVERLAPPEDWINDOW)) {
        return 0;
    }
    ShowWindow(win.Window(), nCmdShow);
    MSG msg = { };
    while (GetMessage(&msg, nullptr,  0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}