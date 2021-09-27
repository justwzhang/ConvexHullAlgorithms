#pragma once
#include <vector>
#include <iterator>
#include <iostream>
#include <d2d1.h>
#include <cmath>
#include "Convexhull.h"
#pragma comment(lib, "d2d1")

using namespace std;

class MinkowskiDifference {
public:
    //used for rendering of the hull
    static void DrawHullAndPoints(
        vector<D2D1_ELLIPSE> hull,
        vector<D2D1_ELLIPSE> listOfTotalPoints,
        ID2D1HwndRenderTarget* pRenderTarget,
        ID2D1SolidColorBrush* pBrushGreen,
        ID2D1SolidColorBrush* pBrushWhite) {

        pRenderTarget->BeginDraw();
        for (int i = 0; i < hull.size(); i++) {
            if (i == hull.size() - 1)
                pRenderTarget->DrawLine(hull[i].point, hull[0].point, pBrushWhite);
            else
                pRenderTarget->DrawLine(hull[i].point, hull[i + 1].point, pBrushWhite);
        }
        for (int i = 0; i < listOfTotalPoints.size(); i++) {
            pRenderTarget->FillEllipse(listOfTotalPoints[i], pBrushGreen);
        }
        pRenderTarget->EndDraw();
    }

    static void DrawHullAndPointsTotal(
        vector<D2D1_ELLIPSE> hull,
        vector<D2D1_ELLIPSE> listOfTotalPoints,
        ID2D1HwndRenderTarget* pRenderTarget,
        ID2D1SolidColorBrush* pBrushRed) {

        pRenderTarget->BeginDraw();
        for (int i = 0; i < hull.size(); i++) {
            if (i == hull.size() - 1)
                pRenderTarget->DrawLine(hull[i].point, hull[0].point, pBrushRed);
            else
                pRenderTarget->DrawLine(hull[i].point, hull[i + 1].point, pBrushRed);
        }
        pRenderTarget->EndDraw();
    }


    //called to get the current hull given 
    /*The hull is returned in this format
    given hull is a vector<D2D1_ELLIPSE> = {1, 2, 3, 4, 5, 6} where the elements represnt points on the convex hull,
    the lines attach points i and i+1 and the last point(in this case is point 6) is attached to the first point(point 1).
    So the lines here will go 1-2-3-4-5-6-1 for this example
    */
    static vector<D2D1_ELLIPSE> GetHull(vector<D2D1_ELLIPSE> fullListOfPoints, ID2D1HwndRenderTarget* pRenderTarget) {
        vector<D2D1_ELLIPSE> hull;
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
        if (NotInHull(hull, farthestRight))
            hull.push_back(farthestRight);
        if (NotInHull(hull, farthestBottom))
            hull.push_back(farthestBottom);
        if (NotInHull(hull, farthestLeft))
            hull.push_back(farthestLeft);
        for (int i = 1; i < hull.size() + 1; i++) {
            if (i != hull.size()) {
                int index = PointFarthestFromEdgeIndex(i - 1, i, hull, fullListOfPoints, pRenderTarget);
                if ((index != -1) && NotInHull(hull, fullListOfPoints[index])) {
                    hull.insert(hull.begin() + i, fullListOfPoints[index]);
                    if (i > 1)
                        i = i - 2;
                    else
                        i--;
                }
            }
            else {
                int index = PointFarthestFromEdgeIndex(hull.size() - 1, 0, hull, fullListOfPoints, pRenderTarget);
                if ((index != -1) && NotInHull(hull, fullListOfPoints[index])) {
                    hull.insert(hull.begin() + i, fullListOfPoints[index]);
                    if (i > 1)
                        i = i - 2;
                    else
                        i--;
                }
            }
        }
        return hull;
    }

    //checks if the point is already in the hull or not
    //can also be used for regular vectors containing ellipses
    static bool NotInHull(vector<D2D1_ELLIPSE> hull, D2D1_ELLIPSE point) {
        for (int i = 0; i < hull.size(); i++) {
            if ((hull[i].point.x == point.point.x) && (hull[i].point.y == point.point.y))
                return false;
        }
        return true;
    }

    //generates a random hull with 6 points (1st hull)
    static vector<D2D1_ELLIPSE> GeneratePointList(ID2D1HwndRenderTarget* pRenderTarget) {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        float minX = size.width / 3;
        float maxX = size.width - minX;
        float maxY = size.height;
        vector<D2D1_ELLIPSE> list;
        for (int i = 1; i <= 6; i++) {
            int randnumX = rand() % 10;
            float randX = (randnumX + 10) * (maxX / 50) + minX;
            int randnumY = rand() % 10;
            float randY = (randnumY + 25) * (maxY / 50);
            D2D1_ELLIPSE temp = D2D1::Ellipse(D2D1::Point2F(randX, randY), 10.0, 10.0);
            list.push_back(temp);
        }
        return list;
    }

    //generates a random hull with 6 points (2nd hull)
    static vector<D2D1_ELLIPSE> GeneratePointList2(ID2D1HwndRenderTarget* pRenderTarget) {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        float minX = size.width / 3;
        float maxX = size.width - minX;
        float maxY = size.height;
        vector<D2D1_ELLIPSE> list;
        for (int i = 1; i <= 6; i++) {
            int randnumX = rand() % 10;
            float randX = (randnumX + 30) * (maxX / 50) + minX;
            int randnumY = rand() % 10;
            float randY = (randnumY + 35) * (maxY / 50);
            D2D1_ELLIPSE temp = D2D1::Ellipse(D2D1::Point2F(randX, randY), 10.0, 10.0);
            list.push_back(temp);
        }
        return list;
    }

    //Generates the points need for the difference hull
    static vector<D2D1_ELLIPSE> GeneratePointListTotal(vector<D2D1_ELLIPSE> list, vector<D2D1_ELLIPSE> list2, ID2D1HwndRenderTarget* pRenderTarget) {
        vector<D2D1_ELLIPSE> listTotal;
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        float minX = size.width / 3;
        float maxX = size.width - minX;
        float maxY = size.height;
        float originX = maxX / 2;
        float originY = maxY / 2;
        //origin = (MaxX/2, MaxY/2)
        for (int i = 0; i < list.size(); i++) {
            for (int j = 0; j < list2.size(); j++) {
                float distOrigX = list[i].point.x - originX;
                float distOrigY = list[i].point.y - originY;
                float distOrigX2 = list2[j].point.x - originX;
                float distOrigY2 = list2[j].point.y - originY;
                float pointXAdded = originY + (distOrigX2 - distOrigX);
                float pointYAdded = originY + (distOrigY - distOrigY2);
                D2D1_ELLIPSE temp = D2D1::Ellipse(D2D1::Point2F(pointXAdded, pointYAdded), 0.0, 0.0);
                listTotal.push_back(temp);
            }
        }
        return listTotal;
    }

    //finds the point farthest from the input line (from point1 to point2), returns -1 if one is not found
    //Look at the tb to understand the forloop if necessary
    static int PointFarthestFromEdgeIndex(int indexOfP1, int indexOfP2, vector<D2D1_ELLIPSE> currentHull, vector<D2D1_ELLIPSE> pointList, ID2D1HwndRenderTarget* pRenderTarget) {
        D2D1_ELLIPSE point1 = currentHull[indexOfP1];
        D2D1_ELLIPSE point2 = currentHull[indexOfP2];
        Vector2D originalVector, perpendicularVector;
        originalVector.xComponent = GetXComponent(point1, point2, pRenderTarget);
        originalVector.yComponent = GetYComponent(point1, point2, pRenderTarget);
        perpendicularVector.xComponent = originalVector.yComponent == 0 ? 0 : originalVector.yComponent * -1;
        perpendicularVector.yComponent = originalVector.xComponent;

        int bestIndex = -1;
        float maxVal = -2;
        float rightMostValue = -2;

        for (int i = 0; i < pointList.size(); i++) {
            Vector2D vectorFromPointiToPoint1;
            vectorFromPointiToPoint1.xComponent = GetXComponent(pointList[i], point1, pRenderTarget);
            vectorFromPointiToPoint1.yComponent = GetYComponent(pointList[i], point1, pRenderTarget);
            Vector2D normOriginal = Vector2D::Normalize(originalVector);
            Vector2D normPerpendicular = Vector2D::Normalize(perpendicularVector);
            Vector2D normVectoriToPoint1 = Vector2D::Normalize(vectorFromPointiToPoint1);
            float d = Vector2D::dotProduct(normVectoriToPoint1, normPerpendicular);
            float r = Vector2D::dotProduct(normVectoriToPoint1, normOriginal);
            if (d > maxVal || (d == maxVal && r > rightMostValue)) {
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
