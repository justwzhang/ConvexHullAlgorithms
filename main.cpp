#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include "basewin.h"

class MainWindow : public BaseWindow<MainWindow> {
    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrush;
    D2D1_RECT_F           rect1;
    D2D1_RECT_F           rect2;
    D2D1_RECT_F           rect3;
    D2D1_RECT_F           rect4;
    D2D1_RECT_F           rect5;
    D2D1_POINT_2F         topLeft;
    D2D1_POINT_2F         topRight;
    D2D1_POINT_2F         bottomLeft;
    D2D1_POINT_2F         bottomRight;
    D2D1_POINT_2F         listLeftTop;
    D2D1_POINT_2F         listLeftBottom;


    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();

public:

    MainWindow() : pFactory(nullptr), pRenderTarget(nullptr), pBrush(nullptr) {}

    PCWSTR  ClassName() const { return L"Circle Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

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

//handels the initialization of the pFactory
HRESULT MainWindow::CreateGraphicsResources() {
    HRESULT hr = S_OK;
    if (pRenderTarget == nullptr) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
        hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hwnd, size), &pRenderTarget);
        if (SUCCEEDED(hr)) {
            const D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::Orange); //light blue
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            if (SUCCEEDED(hr)) {
                CalculateLayout();
            }
        }
    }
    return hr;
}

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

//the onpaint method which redraws the rectangles on the left
void MainWindow::OnPaint() {
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr)) {
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
        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

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

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))){
            return -1;  // Fail CreateWindowEx.
        }
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        OnPaint();
        return 0;

        // Other messages not shown...

    case WM_SIZE:
        Resize();
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    MainWindow win;
    if (!win.Create(L"Convex Hull Algorithms", WS_OVERLAPPEDWINDOW)) {
        return 0;
    }
    ShowWindow(win.Window(), nCmdShow);
    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, nullptr,  0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}