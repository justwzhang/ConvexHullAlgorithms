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
        float magnitude = sqrt((v.xComponent * v.xComponent) + (v.yComponent*v.yComponent));
        v.xComponent = v.xComponent / magnitude;
        v.yComponent = v.yComponent / magnitude;
        return v;
    }
};
//Handles almost all of the quick hull algorithm except for the rendering
class Quickhull{
private:
    //may delete this if I decide to include the rendering in this class
    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrushYellow;
    ID2D1SolidColorBrush* pBrushWhite;
public:
    void Initialize( 
        ID2D1Factory* pFactoryIn, 
        ID2D1HwndRenderTarget* pRenderTargetIn, 
        ID2D1SolidColorBrush* pBrushYellowIn,
        ID2D1SolidColorBrush* pBrushWhiteIn) {


        this->pFactory = pFactoryIn;
        this->pRenderTarget = pRenderTargetIn;
        this->pBrushYellow = pBrushYellowIn;
        this->pBrushWhite = pBrushWhiteIn;
    }
    //called to get the current hull given 
    /*The hull is returned in this format
    given hull is a vector<D2D1_ELLIPSE> = {1, 2, 3, 4, 5, 6} where the elements represnt points on the convex hull,
    the lines attach points i and i+1 and the last point(in this case is point 6) is attached to the first point(point 1).
    So the lines here will go 1-2-3-4-5-6-1 for this example
    */
    vector<D2D1_ELLIPSE> GetHull(vector<D2D1_ELLIPSE> fullListOfPoints) {
        vector<D2D1_ELLIPSE> hull;
        vector<D2D1_ELLIPSE> extremePoints;
        D2D1_ELLIPSE farthestLeft = fullListOfPoints.front();
        D2D1_ELLIPSE farthestTop = fullListOfPoints.front();
        D2D1_ELLIPSE farthestRight = fullListOfPoints.front();
        D2D1_ELLIPSE farthestBottom = fullListOfPoints.front();

        for (int i = 0; i < fullListOfPoints.size(); i++) {
            D2D1_ELLIPSE point = fullListOfPoints[i];
            if (point.point.x < farthestLeft.point.x)
                farthestLeft = point;
            if (point.point.x > farthestRight.point.x)
                farthestRight = point;
            if (point.point.y < farthestTop.point.y)
                farthestTop = point;
            if (point.point.y > farthestBottom.point.y)
                farthestBottom = point;
        }
        hull.push_back(farthestTop);
        hull.push_back(farthestRight);
        hull.push_back(farthestBottom);
        hull.push_back(farthestLeft);

        extremePoints.push_back(farthestTop);
        extremePoints.push_back(farthestRight);
        extremePoints.push_back(farthestBottom);
        extremePoints.push_back(farthestLeft);

        for (int i = 1; i < hull.size() + 1; i++) {
            if (i != hull.size()) {
                int index = PointFarthestFromEdgeIndex(i-1, i, hull, fullListOfPoints);
                if ((index != i || index != i - 1) && (std::find(hull.begin(), hull.end(), fullListOfPoints[index]) == hull.end())) {
                    hull.insert(hull.begin() + i + 1, fullListOfPoints[index]);
                    if (i > 1)
                        i = i - 2;
                    else
                        i--;
                }
            }else {
                int index = PointFarthestFromEdgeIndex(hull.size()-1, 0, hull, fullListOfPoints);
                if ((index != hull.size()-1 || index != 0) && (std::find(hull.begin(), hull.end(), fullListOfPoints[index]) == hull.end())) {
                    hull.insert(hull.begin() + i + 1, fullListOfPoints[index]);
                    if (i > 1)
                        i = i - 2;
                    else
                        i--;
                }
            }
        }
        return hull;
    }
    //finds the point farthest from the input line (from point1 to point2), returns an input index if one is not found
    int PointFarthestFromEdgeIndex(int indexOfP1, int indexOfP2, vector<D2D1_ELLIPSE> currentHull, vector<D2D1_ELLIPSE> pointList) {
        D2D1_ELLIPSE point1 = currentHull[indexOfP1];
        D2D1_ELLIPSE point2 = currentHull[indexOfP2];
        Vector2D originalVector, perpendicularVector;
        originalVector.xComponent = GetXComponent(point1, point2);
        originalVector.yComponent = GetYComponent(point1, point2);
        perpendicularVector.xComponent = originalVector.yComponent == 0 ? 0 : originalVector.yComponent * -1;
        perpendicularVector.yComponent = originalVector.xComponent;

        int bestIndex = indexOfP1;
        float maxVal = -2;
        float rightMostValue = -2;

        for (int i = 0; i < pointList.size(); i++) {
            Vector2D vectorFromPointiToPoint1;
            vectorFromPointiToPoint1.xComponent = GetXComponent(point1, pointList[i]);
            vectorFromPointiToPoint1.yComponent = GetYComponent(point1, pointList[i]);
            Vector2D normOriginal = Vector2D::Normalize(originalVector);
            Vector2D normPerpendicular = Vector2D::Normalize(perpendicularVector);
            Vector2D normVectoriToPoint1 = Vector2D::Normalize(vectorFromPointiToPoint1);
            float d = Vector2D::dotProduct(normVectoriToPoint1, normPerpendicular);
            float r = Vector2D::dotProduct(normVectoriToPoint1, normOriginal);
            if (d > maxVal ||(d == maxVal && r>rightMostValue)) {
                bestIndex = i;
                maxVal = d;
                rightMostValue = r;
            }
        }
        return bestIndex;
    }

    /*These two functions may need to be changed if there is a grid available.
    As of right now, the origin is the center of the alocated area of the right
    and it increments one per pixel.
    */

    //gets the x compnent relative to the center of the right 2/3 of the screen which is the alocated area of the gui
    float GetXComponent(D2D1_ELLIPSE startPt, D2D1_ELLIPSE endPt) {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        float xOrigin = 2 * size.width / 3;
        return ((endPt.point.x-xOrigin) - (startPt.point.x-xOrigin));


    }
    //gets the y compnent relative to the center of the right 2/3 of the screen which is the alocated area of the gui
    float GetYComponent(D2D1_ELLIPSE startPt, D2D1_ELLIPSE endPt) {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        float yOrigin = size.height / 2;
        return ((endPt.point.y - yOrigin) - (startPt.point.y - yOrigin));
    }
    
    
};

