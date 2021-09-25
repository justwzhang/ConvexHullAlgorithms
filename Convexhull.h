#pragma once
#include <vector>
#include <iterator>
#include <iostream>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

using namespace std;
//a basic vector class that can store the x and y components of a vector, compute basic info such as dot product and normalization
class Vector2D {
public:
    float xComponent;
    float yComponent;

    static float dotProduct(Vector2D v1, Vector2D v2) {
        return (v1.xComponent * v2.xComponent) + (v1.yComponent * v2.yComponent);
    }

    static Vector2D Normalize(Vector2D v) {
        float magnitude = sqrt((v.xComponent * v.xComponent) + (v.yComponent * v.yComponent));
        v.xComponent = v.xComponent / magnitude;
        v.yComponent = v.yComponent / magnitude;
        return v;
    }
};

class PointConvexhull {
public:

    //Checks if a point is in a given hull
    static bool Contains(float posX, float posY, vector<D2D1_ELLIPSE> hull, ID2D1HwndRenderTarget* pRenderTarget) {
        D2D1_ELLIPSE testEllipse = D2D1::Ellipse(D2D1::Point2F(posX, posY), 1, 1);
        for (int i = 1; i < hull.size(); i++) {
            if (IsDifferentPoints(hull[i - 1], hull[i]))
                if (!IsPointInside(hull[i - 1], hull[i], testEllipse, pRenderTarget))
                    return false;
        }
        if (IsDifferentPoints(hull[hull.size() - 1], hull[0]))
            if (!IsPointInside(hull[hull.size() - 1], hull[0], testEllipse, pRenderTarget))
                return false;
        return true;
    }

    static bool IsDifferentPoints(D2D1_ELLIPSE point1, D2D1_ELLIPSE point2) {
        if (point1.point.x == point2.point.x && point1.point.y == point2.point.y) {
            return false;
        }
        return true;
    }
    //checks if a point is on the right side of a given side from point1 to point2
    static bool IsPointInside(D2D1_ELLIPSE point1, D2D1_ELLIPSE point2, D2D1_ELLIPSE testedPoint, ID2D1HwndRenderTarget* pRenderTarget) {
        Vector2D originalVector, perpendicularVector;
        originalVector.xComponent = GetXComponent(point1, point2, pRenderTarget);
        originalVector.yComponent = GetYComponent(point1, point2, pRenderTarget);
        perpendicularVector.xComponent = originalVector.yComponent == 0 ? 0 : originalVector.yComponent * -1;
        perpendicularVector.yComponent = originalVector.xComponent;


        Vector2D vectorFromPointTestedToPoint1;
        vectorFromPointTestedToPoint1.xComponent = GetXComponent(testedPoint, point1, pRenderTarget);
        vectorFromPointTestedToPoint1.yComponent = GetYComponent(testedPoint, point1, pRenderTarget);
        Vector2D normPerpendicular = Vector2D::Normalize(perpendicularVector);
        Vector2D normVectorTestedToPoint1 = Vector2D::Normalize(vectorFromPointTestedToPoint1);
        float dotProduct = Vector2D::dotProduct(normPerpendicular, normVectorTestedToPoint1);

        return dotProduct >= 0 ? true : false;
    }

    /*These two functions may need to be changed if there is a grid available.
    As of right now, the origin is the center of the alocated area of the right
    and it increments one per pixel.
    */

    //gets the x compnent relative to the center of the right 2/3 of the screen which is the alocated area of the gui
    static float GetXComponent(D2D1_ELLIPSE startPt, D2D1_ELLIPSE endPt, ID2D1HwndRenderTarget* pRenderTarget) {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        float xOrigin = 2 * (size.width / 3);
        return ((endPt.point.x - xOrigin) - (startPt.point.x - xOrigin));


    }
    //gets the y compnent relative to the center of the right 2/3 of the screen which is the alocated area of the gui
    static float GetYComponent(D2D1_ELLIPSE startPt, D2D1_ELLIPSE endPt, ID2D1HwndRenderTarget* pRenderTarget) {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        float yOrigin = size.height / 2;
        return ((endPt.point.y - yOrigin) - (startPt.point.y - yOrigin));
    }

};