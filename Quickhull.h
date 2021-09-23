#pragma once
#include <list>
#include <iostream>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

using namespace std;

class Quickhull{
private:
	list<D2D1_ELLIPSE> fullListOfPoints;
	list<D2D1_ELLIPSE> hull;
    list<D2D1_ELLIPSE> extremePoints;

    D2D1_POINT_2F farthestLeft;
    D2D1_POINT_2F farthestRight;
    D2D1_POINT_2F farthestTop;
    D2D1_POINT_2F farthestBottom;

    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrush;
    ID2D1SolidColorBrush* pBrushYellow;
    ID2D1SolidColorBrush* pBrushWhite;
public:
    void temp() {
        
    }

};