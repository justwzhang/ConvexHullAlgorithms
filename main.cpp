#include <windows.h>
#include <Windowsx.h>
#include <math.h>
#include <dwrite.h>
#include <iostream>
#pragma comment(lib, "Dwrite")

#include "basewin.h"
#include "Quickhull.h"
#include "MinkowskiSum.h"
#include "MinkowskiDifference.h"
#include "GJK.h"


class MainWindow : public BaseWindow<MainWindow> {
    int          currentAlgLoaded;
    //tools for writing the text in the boxes
    IDWriteFactory* pWriteFactory;
    ID2D1SolidColorBrush* pTextBrush;
    IDWriteTextFormat* pTextFormat;
    //tools for drawing everything
    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    D2D1_POINT_2F oldMousePos;

    ID2D1SolidColorBrush* pBrush;
    ID2D1SolidColorBrush* pBrushYellow;
    ID2D1SolidColorBrush* pBrushWhite;
    ID2D1SolidColorBrush* pBrushRed;
    ID2D1SolidColorBrush* pBrushGreen;
    ID2D1SolidColorBrush* pBrushPurple;
    ID2D1SolidColorBrush* pBrushGray;
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
    D2D1_POINT_2F         lineTop;
    D2D1_POINT_2F         lineBottom;
    D2D1_POINT_2F         lineLeft;
    D2D1_POINT_2F         lineRight;
    //usefull objects for each algorithm
    //quickhull
    vector<D2D1_ELLIPSE> quickhullListOFPointsForHull;
    vector<D2D1_ELLIPSE> quickhullPointList;
    //point convexhull
    vector<D2D1_ELLIPSE> convexhullListOFPointsForHull;
    D2D1_ELLIPSE         targetPoint;
    //min sum
    vector<D2D1_ELLIPSE> minSumListOFPointsForHull;
    vector<D2D1_ELLIPSE> minSumPointList;
    vector<D2D1_ELLIPSE> minSumListOFPointsForHull2;
    vector<D2D1_ELLIPSE> minSumPointList2;
    vector<D2D1_ELLIPSE> minSumListOFPointsForHullTotal;
    vector<D2D1_ELLIPSE> minSumPointListTotal;
    //min diff
    vector<D2D1_ELLIPSE> minDiffListOFPointsForHull;
    vector<D2D1_ELLIPSE> minDiffPointList;
    vector<D2D1_ELLIPSE> minDiffListOFPointsForHull2;
    vector<D2D1_ELLIPSE> minDiffPointList2;
    vector<D2D1_ELLIPSE> minDiffListOFPointsForHullTotal;
    vector<D2D1_ELLIPSE> minDiffPointListTotal;
    //gjk
    vector<D2D1_ELLIPSE> gjkListOFPointsForHull;
    vector<D2D1_ELLIPSE> gjkPointList;
    vector<D2D1_ELLIPSE> gjkListOFPointsForHull2;
    vector<D2D1_ELLIPSE> gjkPointList2;
    vector<D2D1_ELLIPSE> gjkListOFPointsForHullTotal;
    vector<D2D1_ELLIPSE> gjkPointListTotal;
    bool intersect;

    bool hullHit;
    bool pointHit;
    bool hasHit;
    int selectionIndex;
    int currentHullHit;

    BOOL    HitDetectEllipse(int mouseX, int mouseY, vector<D2D1_ELLIPSE> listOfPoints);
    BOOL    HitDetectHull(int mouseX, int mouseY, vector<D2D1_ELLIPSE> hull);
    BOOL    IsInRect(int mouseX, int mouseY, D2D1_RECT_F rect);
    void    OnLButtonDown(int pixelX, int pixelY);

    float Distance(int mouseX, int mouseY, int ellipseX, int ellipseY);
    void    CalculateLayout();
    void    MakeGrid();
    HRESULT CreateTextResources();
    HRESULT CreateGraphicsResources();
    void    DiscardTextResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();
    void    OnMouseMove(int pixelX, int pixelY, DWORD flags);
    void    OnMouseUp();
    vector<D2D1_ELLIPSE> MoveHull(int dipX, int dipY, vector<D2D1_ELLIPSE>hull);

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

void MainWindow::MakeGrid() {
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

        topLeft = D2D1::Point2F(0, 0);
        topRight = D2D1::Point2F(size.width, 0);
        bottomLeft = D2D1::Point2F(0, size.height);
        bottomRight = D2D1::Point2F(size.width, size.height);

        listLeftTop = D2D1::Point2F(farthestX, 0);
        listLeftBottom = D2D1::Point2F(farthestX, size.height);

        int j = 0;
        pRenderTarget->BeginDraw();
        for (float i = farthestX; i <= size.width; i += (size.width - farthestX) / 50) {
            lineTop = D2D1::Point2F(i, 0);
            lineBottom = D2D1::Point2F(i, size.height);
            if (j == 25)
                pRenderTarget->DrawLine(lineTop, lineBottom, pBrush);
            else
                pRenderTarget->DrawLine(lineTop, lineBottom, pBrushGray);
            j++;
        }
        pRenderTarget->EndDraw();

        j = 0;
        pRenderTarget->BeginDraw();
        for (float i = 0; i <= size.height; i += size.height/ 50) {
            lineLeft = D2D1::Point2F(farthestX, i);
            lineRight = D2D1::Point2F(size.width, i);
            if (j == 25)
                pRenderTarget->DrawLine(lineLeft, lineRight, pBrush);
            else
                pRenderTarget->DrawLine(lineLeft, lineRight, pBrushGray);
            j++;
        }
        pRenderTarget->EndDraw();
    }
}

//handels the initialization of the pRenderTarget
HRESULT MainWindow::CreateGraphicsResources() {
    HRESULT hr = S_OK;
    HRESULT hr2 = S_OK;
    HRESULT hr3 = S_OK;
    HRESULT hr4 = S_OK;
    HRESULT hr5 = S_OK;
    HRESULT hr6 = S_OK;
    HRESULT hr7 = S_OK;
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
            hr5 = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pBrushRed);
            hr6 = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &pBrushGreen);
            hr6 = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Purple), &pBrushPurple);
            hr7 = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &pBrushGray);

            if (SUCCEEDED(hr) && SUCCEEDED(hr2) && SUCCEEDED(hr3) && SUCCEEDED(hr4) && SUCCEEDED(hr5) && SUCCEEDED(hr6) && SUCCEEDED(hr7)) {
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
    SafeRelease(&pBrushGreen);
    SafeRelease(&pBrushPurple);
    SafeRelease(&pBrushRed);
    SafeRelease(&pBrushWhite);
    SafeRelease(&pBrushYellow);
    SafeRelease(&pBrushGray);
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
    /*if (currentAlgLoaded == L"quickhull") {
        Quickhull::DrawHullAndPoints(quickhullListOFPointsForHull, quickhullPointList, pRenderTarget, pBrushYellow, pBrushWhite);
    }*/
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

float MainWindow::Distance(int mouseX, int mouseY, int ellipseX, int ellipseY) {
    return sqrt(((ellipseX - mouseX) * (ellipseX - mouseX)) + ((ellipseY - mouseY) * (ellipseY - mouseY)));
}

BOOL MainWindow::HitDetectEllipse(int mouseX, int mouseY, vector<D2D1_ELLIPSE> list) {
    const float dipX = DPIScale::PixelsToDipsX(mouseX);
    const float dipY = DPIScale::PixelsToDipsY(mouseY);
    for (int i = 0; i < list.size(); i++) {
        const float a = list[i].radiusX;
        const float b = list[i].radiusY;
        const float x1 = dipX - list[i].point.x;
        const float y1 = dipY - list[i].point.y;
        //const float d = ((x1 * x1) / (a * a)) + ((y1 * y1) / (b * b));
        //if (d <= 1.0f) {
        if(Distance(dipX,dipY,list[i].point.x,list[i].point.y)< a){
            selectionIndex = i;
            pointHit = true;
            return true;
        }
    }
    hasHit = true;
    return false;
}

BOOL MainWindow::HitDetectHull(int mouseX, int mouseY, vector<D2D1_ELLIPSE> hull) {
    const float dipX = DPIScale::PixelsToDipsX(mouseX);
    const float dipY = DPIScale::PixelsToDipsY(mouseY);
    bool temp = PointConvexhull::Contains(dipX, dipY, hull, pRenderTarget);
    if (temp)
        hullHit = true;
    hasHit = true;
    return temp;
}

//handles the left button clicks
void MainWindow::OnLButtonDown(int pixelX, int pixelY) {
    D2D1_SIZE_F size = pRenderTarget->GetSize();
    const float dipX = DPIScale::PixelsToDipsX(pixelX);
    const float dipY = DPIScale::PixelsToDipsY(pixelY);
    oldMousePos.x = dipX;
    oldMousePos.y = dipY;
    if (IsInRect(pixelX, pixelY, rect1)) {
        currentAlgLoaded = 1;
        OnPaint();
        MakeGrid();
        minDiffPointList = MinkowskiDifference::GeneratePointList(pRenderTarget);
        minDiffListOFPointsForHull = MinkowskiDifference::GetHull(minDiffPointList, pRenderTarget);
        MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull, minDiffPointList, pRenderTarget, pBrushGreen, pBrushWhite);
        minDiffPointList2 = MinkowskiDifference::GeneratePointList2(pRenderTarget);
        minDiffListOFPointsForHull2 = MinkowskiDifference::GetHull(minDiffPointList2, pRenderTarget);
        MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull2, minDiffPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
        minDiffPointListTotal = MinkowskiDifference::GeneratePointListTotal(minDiffPointList, minDiffPointList2, pRenderTarget);
        minDiffListOFPointsForHullTotal = MinkowskiDifference::GetHull(minDiffPointListTotal, pRenderTarget);
        MinkowskiDifference::DrawHullAndPointsTotal(minDiffListOFPointsForHullTotal, minDiffPointListTotal, pRenderTarget, pBrushRed);
        //Minkowski Difference
    }
    else if (IsInRect(pixelX, pixelY, rect2)) {
        currentAlgLoaded = 2;
        OnPaint();
        MakeGrid();
        minSumPointList = MinkowskiSum::GeneratePointList(pRenderTarget);
        minSumListOFPointsForHull = MinkowskiSum::GetHull(minSumPointList, pRenderTarget);
        MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull, minSumPointList, pRenderTarget, pBrushGreen, pBrushWhite);
        minSumPointList2 = MinkowskiSum::GeneratePointList2(pRenderTarget);
        minSumListOFPointsForHull2 = MinkowskiSum::GetHull(minSumPointList2, pRenderTarget);
        MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull2, minSumPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
        minSumPointListTotal = MinkowskiSum::GeneratePointListTotal(minSumPointList, minSumPointList2, pRenderTarget);
        minSumListOFPointsForHullTotal = MinkowskiSum::GetHull(minSumPointListTotal, pRenderTarget);
        MinkowskiSum::DrawHullAndPointsTotal(minSumListOFPointsForHullTotal, minSumPointListTotal, pRenderTarget, pBrushRed);
        //Minkowski Sum
    }
    else if (IsInRect(pixelX, pixelY, rect3)) {
        currentAlgLoaded = 3;
        OnPaint();
        quickhullPointList = Quickhull::GeneratePointList(pRenderTarget);
        quickhullListOFPointsForHull = Quickhull::GetHull(quickhullPointList, pRenderTarget);
        Quickhull::DrawHullAndPoints(quickhullListOFPointsForHull, quickhullPointList, pRenderTarget, pBrushYellow, pBrushWhite);
        //Quickhull
    }
    else if (IsInRect(pixelX, pixelY, rect4)) {
        currentAlgLoaded = 4;
        OnPaint();
        convexhullListOFPointsForHull = PointConvexhull::CreateHull(pRenderTarget);
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        targetPoint = D2D1::Ellipse(D2D1::Point2F(2*size.width/3, size.height/2), 10, 10);
        PointConvexhull::DrawHullAndPoints(convexhullListOFPointsForHull, targetPoint, pRenderTarget, pBrushGreen, pBrushWhite, pBrushRed);
        //Point Convex Hull
    }
    else if (IsInRect(pixelX, pixelY, rect5)) {
        currentAlgLoaded = 5;
        OnPaint();
        MakeGrid();
        gjkPointList = GJK::GeneratePointList(pRenderTarget);
        gjkListOFPointsForHull = GJK::GetHull(gjkPointList, pRenderTarget);
        GJK::DrawHullAndPoints(gjkListOFPointsForHull, gjkPointList, pRenderTarget, pBrushGreen, pBrushWhite);
        gjkPointList2 = GJK::GeneratePointList2(pRenderTarget);
        gjkListOFPointsForHull2 = GJK::GetHull(gjkPointList2, pRenderTarget);
        GJK::DrawHullAndPoints(gjkListOFPointsForHull2, gjkPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
        //intersection comparison
        intersect = GJK::Intersects(gjkListOFPointsForHull, gjkListOFPointsForHull2, pRenderTarget);
        gjkPointListTotal = GJK::GeneratePointListTotal(gjkPointList, gjkPointList2, pRenderTarget);
        gjkListOFPointsForHullTotal = GJK::GetHull(gjkPointListTotal, pRenderTarget);
        GJK::DrawHullAndPointsTotal(gjkListOFPointsForHullTotal, gjkPointListTotal, pRenderTarget, pBrushGreen, pBrushPurple, intersect);
        //Gjk
    }


}

void MainWindow::OnMouseMove(int pixelX, int pixelY, DWORD flags) {
    const float dipX = DPIScale::PixelsToDipsX(pixelX);
    const float dipY = DPIScale::PixelsToDipsY(pixelY);
    if (currentAlgLoaded == 1) {//mindiff
        if (HitDetectEllipse(pixelX, pixelY, minDiffPointList) && !hullHit) {
            if ((flags & MK_LBUTTON) && selectionIndex != -1) {
                minDiffPointList[selectionIndex].point.x = dipX;
                minDiffPointList[selectionIndex].point.y = dipY;
                minDiffListOFPointsForHull = MinkowskiDifference::GetHull(minDiffPointList, pRenderTarget);
                OnPaint();
                MakeGrid();
                MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull, minDiffPointList, pRenderTarget, pBrushGreen, pBrushWhite);
                MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull2, minDiffPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
                minDiffPointListTotal = MinkowskiDifference::GeneratePointListTotal(minDiffPointList, minDiffPointList2, pRenderTarget);
                minDiffListOFPointsForHullTotal = MinkowskiDifference::GetHull(minDiffPointListTotal, pRenderTarget);
                MinkowskiDifference::DrawHullAndPointsTotal(minDiffListOFPointsForHullTotal, minDiffPointListTotal, pRenderTarget, pBrushRed);
            }
        }
        if (HitDetectEllipse(pixelX, pixelY, minDiffPointList2) && !hullHit) {
            if ((flags & MK_LBUTTON) && selectionIndex != -1) {
                minDiffPointList2[selectionIndex].point.x = dipX;
                minDiffPointList2[selectionIndex].point.y = dipY;
                minDiffListOFPointsForHull2 = MinkowskiDifference::GetHull(minDiffPointList2, pRenderTarget);
                OnPaint();
                MakeGrid();
                MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull, minDiffPointList, pRenderTarget, pBrushGreen, pBrushWhite);
                MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull2, minDiffPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
                minDiffPointListTotal = MinkowskiDifference::GeneratePointListTotal(minDiffPointList, minDiffPointList2, pRenderTarget);
                minDiffListOFPointsForHullTotal = MinkowskiDifference::GetHull(minDiffPointListTotal, pRenderTarget);
                MinkowskiDifference::DrawHullAndPointsTotal(minDiffListOFPointsForHullTotal, minDiffPointListTotal, pRenderTarget, pBrushRed);
            }
        }
        if ((flags & MK_LBUTTON) && (HitDetectHull(pixelX, pixelY, minDiffListOFPointsForHull)||currentHullHit == 1) && selectionIndex == -1) {
            currentHullHit == 1;
            minDiffPointList = MoveHull(dipX, dipY, minDiffPointList);
            OnPaint();
            MakeGrid();
            minDiffListOFPointsForHull = MinkowskiDifference::GetHull(minDiffPointList, pRenderTarget);
            MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull, minDiffPointList, pRenderTarget, pBrushGreen, pBrushWhite);
            MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull2, minDiffPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
            
            minDiffPointListTotal = MinkowskiDifference::GeneratePointListTotal(minDiffPointList, minDiffPointList2, pRenderTarget);
            minDiffListOFPointsForHullTotal = MinkowskiDifference::GetHull(minDiffPointListTotal, pRenderTarget);
            MinkowskiDifference::DrawHullAndPointsTotal(minDiffListOFPointsForHullTotal, minDiffPointListTotal, pRenderTarget, pBrushRed);

        }
        if ((flags & MK_LBUTTON) && (HitDetectHull(pixelX, pixelY, minDiffListOFPointsForHull2)||currentHullHit == 2) && selectionIndex == -1) {
            currentHullHit = 2;
            minDiffPointList2 = MoveHull(dipX, dipY, minDiffPointList2);
            OnPaint();
            MakeGrid();
            minDiffListOFPointsForHull2 = MinkowskiDifference::GetHull(minDiffPointList2, pRenderTarget);
            MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull, minDiffPointList, pRenderTarget, pBrushGreen, pBrushWhite);
            MinkowskiDifference::DrawHullAndPoints(minDiffListOFPointsForHull2, minDiffPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
            
            minDiffPointListTotal = MinkowskiDifference::GeneratePointListTotal(minDiffPointList, minDiffPointList2, pRenderTarget);
            minDiffListOFPointsForHullTotal = MinkowskiDifference::GetHull(minDiffPointListTotal, pRenderTarget);
            MinkowskiDifference::DrawHullAndPointsTotal(minDiffListOFPointsForHullTotal, minDiffPointListTotal, pRenderTarget, pBrushRed);

        }
    }
    else if (currentAlgLoaded == 2) {//minsum
        if (HitDetectEllipse(pixelX, pixelY, minSumPointList)) {
            if ((flags & MK_LBUTTON) && selectionIndex != -1) {
                minSumPointList[selectionIndex].point.x = dipX;
                minSumPointList[selectionIndex].point.y = dipY;
                minSumListOFPointsForHull = MinkowskiSum::GetHull(minSumPointList, pRenderTarget);
                OnPaint();
                MakeGrid();
                MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull, minSumPointList, pRenderTarget, pBrushGreen, pBrushWhite);
                MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull2, minSumPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
                minSumPointListTotal = MinkowskiSum::GeneratePointListTotal(minSumPointList, minSumPointList2, pRenderTarget);
                minSumListOFPointsForHullTotal = MinkowskiSum::GetHull(minSumPointListTotal, pRenderTarget);
                MinkowskiSum::DrawHullAndPointsTotal(minSumListOFPointsForHullTotal, minSumPointListTotal, pRenderTarget, pBrushRed);
            }
        }
        if (HitDetectEllipse(pixelX, pixelY, minSumPointList2)) {
            if ((flags & MK_LBUTTON) && selectionIndex != -1) {
                minSumPointList2[selectionIndex].point.x = dipX;
                minSumPointList2[selectionIndex].point.y = dipY;
                minSumListOFPointsForHull2 = MinkowskiSum::GetHull(minSumPointList2, pRenderTarget);
                OnPaint();
                MakeGrid();
                MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull, minSumPointList, pRenderTarget, pBrushGreen, pBrushWhite);
                MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull2, minSumPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
                minSumPointListTotal = MinkowskiSum::GeneratePointListTotal(minSumPointList, minSumPointList2, pRenderTarget);
                minSumListOFPointsForHullTotal = MinkowskiSum::GetHull(minSumPointListTotal, pRenderTarget);
                MinkowskiSum::DrawHullAndPointsTotal(minSumListOFPointsForHullTotal, minSumPointListTotal, pRenderTarget, pBrushRed);
            }
        }
        if ((flags & MK_LBUTTON) && (HitDetectHull(pixelX, pixelY, minSumListOFPointsForHull)||currentHullHit == 1) && selectionIndex == -1) {
            currentHullHit == 1;
            minSumPointList = MoveHull(dipX, dipY, minSumPointList);
            OnPaint();
            MakeGrid();
            minSumListOFPointsForHull = MinkowskiSum::GetHull(minSumPointList, pRenderTarget);
            MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull, minSumPointList, pRenderTarget, pBrushGreen, pBrushWhite);
            MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull2, minSumPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
            
            minSumPointListTotal = MinkowskiSum::GeneratePointListTotal(minSumPointList, minSumPointList2, pRenderTarget);
            minSumListOFPointsForHullTotal = MinkowskiSum::GetHull(minSumPointListTotal, pRenderTarget);
            MinkowskiSum::DrawHullAndPointsTotal(minSumListOFPointsForHullTotal, minSumPointListTotal, pRenderTarget, pBrushRed);

        }
        if ((flags & MK_LBUTTON) && (HitDetectHull(pixelX, pixelY, minSumListOFPointsForHull2)||currentHullHit == 2) && selectionIndex == -1) {
            currentHullHit = 2;
            minSumPointList2 = MoveHull(dipX, dipY, minSumPointList2);
            OnPaint();
            MakeGrid();
            minSumListOFPointsForHull2 = MinkowskiSum::GetHull(minSumPointList2, pRenderTarget);
            MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull, minSumPointList, pRenderTarget, pBrushGreen, pBrushWhite);
            MinkowskiSum::DrawHullAndPoints(minSumListOFPointsForHull2, minSumPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
            
            minSumPointListTotal = MinkowskiSum::GeneratePointListTotal(minSumPointList, minSumPointList2, pRenderTarget);
            minSumListOFPointsForHullTotal = MinkowskiSum::GetHull(minSumPointListTotal, pRenderTarget);
            MinkowskiSum::DrawHullAndPointsTotal(minSumListOFPointsForHullTotal, minSumPointListTotal, pRenderTarget, pBrushRed);
        }
    }
    else if (currentAlgLoaded == 3) {//quickhull
        if (HitDetectEllipse(pixelX, pixelY, quickhullPointList)) {
            if ((flags & MK_LBUTTON) && selectionIndex != -1) {
                quickhullPointList[selectionIndex].point.x = dipX;
                quickhullPointList[selectionIndex].point.y = dipY;
                OnPaint();
                quickhullListOFPointsForHull = Quickhull::GetHull(quickhullPointList, pRenderTarget);
                Quickhull::DrawHullAndPoints(quickhullListOFPointsForHull, quickhullPointList, pRenderTarget, pBrushYellow, pBrushWhite);
            }  
        }
        if ((flags & MK_LBUTTON) && (HitDetectHull(pixelX, pixelY, quickhullListOFPointsForHull) || hullHit) && selectionIndex == -1) {
            quickhullPointList = MoveHull(dipX, dipY, quickhullPointList);
            quickhullListOFPointsForHull = Quickhull::GetHull(quickhullPointList, pRenderTarget);
            OnPaint();
            Quickhull::DrawHullAndPoints(quickhullListOFPointsForHull, quickhullPointList, pRenderTarget, pBrushYellow, pBrushWhite);

        }
    }
    else if (currentAlgLoaded == 4) {//convexhull
        vector<D2D1_ELLIPSE> temp;
        temp.push_back(targetPoint);
        if (HitDetectEllipse(pixelX, pixelY, temp)) {
            if ((flags & MK_LBUTTON) && selectionIndex != -1) {
                targetPoint.point.x = dipX;
                targetPoint.point.y = dipY;
                OnPaint();
                PointConvexhull::DrawHullAndPoints(convexhullListOFPointsForHull, targetPoint, pRenderTarget, pBrushGreen, pBrushWhite, pBrushRed);
            }
        }
        else if ((flags & MK_LBUTTON) &&(HitDetectHull(pixelX, pixelY, convexhullListOFPointsForHull)||hullHit) && selectionIndex == -1) {
            convexhullListOFPointsForHull = MoveHull(dipX, dipY, convexhullListOFPointsForHull);
            OnPaint();
            PointConvexhull::DrawHullAndPoints(convexhullListOFPointsForHull, targetPoint, pRenderTarget, pBrushGreen, pBrushWhite, pBrushRed);

        }
    }
    else if (currentAlgLoaded ==5 ) {
        if (HitDetectEllipse(pixelX, pixelY, gjkPointList)) {
            if ((flags & MK_LBUTTON) && selectionIndex != -1) {
                gjkPointList[selectionIndex].point.x = dipX;
                gjkPointList[selectionIndex].point.y = dipY;
                gjkListOFPointsForHull = GJK::GetHull(gjkPointList, pRenderTarget);
                OnPaint();
                MakeGrid();
                GJK::DrawHullAndPoints(gjkListOFPointsForHull, gjkPointList, pRenderTarget, pBrushGreen, pBrushWhite);
                GJK::DrawHullAndPoints(gjkListOFPointsForHull2, gjkPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
                gjkPointListTotal = GJK::GeneratePointListTotal(gjkPointList, gjkPointList2, pRenderTarget);
                gjkListOFPointsForHullTotal = GJK::GetHull(gjkPointListTotal, pRenderTarget);
                intersect = GJK::Intersects(gjkListOFPointsForHull, gjkListOFPointsForHull2, pRenderTarget);
                GJK::DrawHullAndPointsTotal(gjkListOFPointsForHullTotal, gjkPointListTotal, pRenderTarget, pBrushGreen, pBrushPurple, intersect);
            }
        }
        if (HitDetectEllipse(pixelX, pixelY, gjkPointList2)) {
            if ((flags & MK_LBUTTON) && selectionIndex != -1) {
                gjkPointList2[selectionIndex].point.x = dipX;
                gjkPointList2[selectionIndex].point.y = dipY;
                gjkListOFPointsForHull2 = GJK::GetHull(gjkPointList2, pRenderTarget);
                OnPaint();
                MakeGrid();
                GJK::DrawHullAndPoints(gjkListOFPointsForHull, gjkPointList, pRenderTarget, pBrushGreen, pBrushWhite);
                GJK::DrawHullAndPoints(gjkListOFPointsForHull2, gjkPointList2, pRenderTarget, pBrushGreen, pBrushWhite);
                gjkPointListTotal = GJK::GeneratePointListTotal(gjkPointList, gjkPointList2, pRenderTarget);
                gjkListOFPointsForHullTotal = GJK::GetHull(gjkPointListTotal, pRenderTarget);
                intersect = GJK::Intersects(gjkListOFPointsForHull, gjkListOFPointsForHull2, pRenderTarget);
                GJK::DrawHullAndPointsTotal(gjkListOFPointsForHullTotal, gjkPointListTotal, pRenderTarget, pBrushGreen, pBrushPurple, intersect);
            }
        }
        if ((flags & MK_LBUTTON) && (HitDetectHull(pixelX, pixelY, gjkListOFPointsForHull)||currentHullHit==1) && selectionIndex == -1) {
            currentHullHit = 1;
            gjkPointList = MoveHull(dipX, dipY, gjkPointList);
            OnPaint();
            MakeGrid();
            gjkListOFPointsForHull = GJK::GetHull(gjkPointList, pRenderTarget);
            GJK::DrawHullAndPoints(gjkListOFPointsForHull, gjkPointList, pRenderTarget, pBrushGreen, pBrushWhite);
            GJK::DrawHullAndPoints(gjkListOFPointsForHull2, gjkPointList2, pRenderTarget, pBrushGreen, pBrushWhite);

            gjkPointListTotal = GJK::GeneratePointListTotal(gjkPointList, gjkPointList2, pRenderTarget);
            gjkListOFPointsForHullTotal = GJK::GetHull(gjkPointListTotal, pRenderTarget);
            intersect = GJK::Intersects(gjkListOFPointsForHull, gjkListOFPointsForHull2, pRenderTarget);
            GJK::DrawHullAndPointsTotal(gjkListOFPointsForHullTotal, gjkPointListTotal, pRenderTarget, pBrushGreen, pBrushPurple, intersect);

        }
        if ((flags & MK_LBUTTON) && (HitDetectHull(pixelX, pixelY, gjkListOFPointsForHull2)||currentHullHit ==2) && selectionIndex == -1) {
            currentHullHit = 2;
            gjkPointList2 = MoveHull(dipX, dipY, gjkPointList2);
            OnPaint();
            MakeGrid();
            gjkListOFPointsForHull2 = GJK::GetHull(gjkPointList2, pRenderTarget);
            GJK::DrawHullAndPoints(gjkListOFPointsForHull, gjkPointList, pRenderTarget, pBrushGreen, pBrushWhite);
            GJK::DrawHullAndPoints(gjkListOFPointsForHull2, gjkPointList2, pRenderTarget, pBrushGreen, pBrushWhite);

            gjkPointListTotal = GJK::GeneratePointListTotal(gjkPointList, gjkPointList2, pRenderTarget);
            gjkListOFPointsForHullTotal = GJK::GetHull(gjkPointListTotal, pRenderTarget);
            intersect = GJK::Intersects(gjkListOFPointsForHull, gjkListOFPointsForHull2, pRenderTarget);
            GJK::DrawHullAndPointsTotal(gjkListOFPointsForHullTotal, gjkPointListTotal, pRenderTarget, pBrushGreen, pBrushPurple, intersect);

        }
    }
       
    
}

vector<D2D1_ELLIPSE> MainWindow::MoveHull(int dipX, int dipY, vector<D2D1_ELLIPSE>hull) {
    float distanceX = dipX - oldMousePos.x;
    float distanceY = dipY - oldMousePos.y;
    for (int i = 0; i < hull.size(); i++) {
        hull[i].point.x = hull[i].point.x += distanceX;
        hull[i].point.y = hull[i].point.y += distanceY;
    }
    oldMousePos.x = dipX;
    oldMousePos.y = dipY;
    return hull;
}

void MainWindow::OnMouseUp() {
    selectionIndex = -1;
    currentHullHit = 0;
    hullHit = false;
    pointHit = false;
    hasHit = false;
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
    case WM_LBUTTONUP:
        OnMouseUp();
        return 0;
    case WM_MOUSEMOVE:
        OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(LoadCursor(NULL, IDC_HAND));
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