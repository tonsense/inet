//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "Polygon.h"

namespace inet {

Polygon::Polygon(const std::vector<Coord>& points)
{
    if (points.size() < 3)
        throw cRuntimeError("A Euclidean polygon has at least three points");
    this->points = points;
}

Coord Polygon::getNormalUnitVector() const
{
    Coord point1 = points[0];
    Coord point2 = points[1];
    Coord point3 = points[2];
    Coord vectorA = point2 - point1;
    Coord vectorB = point3 - point1;
    Coord vectorC(vectorA.y * vectorB.z - vectorA.z * vectorB.y,
                 vectorA.z * vectorB.x - vectorA.x * vectorB.z,
                 vectorA.x * vectorB.y - vectorA.y * vectorB.x);
    return vectorC / vectorC.length();
}

Coord Polygon::computeSize() const
{
    Coord min;
    Coord max;
    for (std::vector<Coord>::const_iterator it = points.begin(); it != points.end(); it++) {
        min = min.min(*it);
        max = max.max(*it);
    }
    return max - min;
}

Coord Polygon::getEdgeOutwardNormalVector(const Coord& edgeP1, const Coord& edgeP2) const
{
    Coord polygonNormal = getNormalUnitVector();
    Coord vectorA = edgeP1 - polygonNormal;
    Coord vectorB = edgeP2 - polygonNormal;
    Coord vectorC(vectorA.y * vectorB.z - vectorA.z * vectorB.y,
                 vectorA.z * vectorB.x - vectorA.x * vectorB.z,
                 vectorA.x * vectorB.y - vectorA.y * vectorB.x);
    // The projection of a vector image v onto a plane with unit normal vector n is: p = v - (v*n)*n.
    return vectorC - polygonNormal * (vectorC * polygonNormal);
}

bool Polygon::computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const
{
    // Note: based on http://geomalgorithms.com/a13-_intersect-4.html
    Coord p0 = lineSegment.getPoint1();
    Coord p1 = lineSegment.getPoint2();
    Coord segmentDirection = p1 - p0;
    double tE = 0;
    double tL = 1;
    unsigned int pointSize = points.size();
    for (unsigned int i = 0; i < pointSize; i++)
    {
        Coord normalVec = getEdgeOutwardNormalVector(points[i], points[(i+1) % pointSize]);
        double N = normalVec * (points[i] - p0);
        double D = normalVec * segmentDirection;
        if (D < 0)
        {
            double t = N / D;
            if (t > tE)
            {
                tE = t;
                normal1 = normalVec;
                if (tE > tL)
                    return false;
            }
        }
        else if (D > 0)
        {
            double t = N / D;
            if (t < tL)
            {
                tL = t;
                normal2 = normalVec;
                if (tL < tE)
                    return false;
            }
        }
        else
        {
            if (N < 0)
                return false;
        }
    }
    if (tE == 0)
        normal1 = Coord(0,0,0);
    if (tL == 1)
        normal2 = Coord(0,0,0);
    intersection1 = p0 + segmentDirection * tE;
    intersection2 = p0 + segmentDirection * tL;
    return true;
}

} // namespace inet
