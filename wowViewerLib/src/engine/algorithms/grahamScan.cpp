//Adapted from https://github.com/kartikkukreja/blog-codes/blob/master/src/Graham%20Scan%20Convex%20Hull.cpp
#include "grahamScan.h"
#include <algorithm>
#include "mathHelper.h"
using namespace std;

// returns -1 if a -> b -> c forms a counter-clockwise turn,
// +1 for a clockwise turn, 0 if they are collinear
int ccw(const Point &a, const Point &b, const Point &c) {
    float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (area > 0)
        return -1;
    else if (area < 0)
        return 1;
    return 0;
}

// returns square of Euclidean distance between two points
float sqrDist(Point &a, Point &b)  {
    float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// used for sorting points according to polar order w.r.t the pivot
//bool POLAR_ORDER(Point a, Point b)  {
//    float order = ccw(pivot, a, b);
//    if (order == 0)
//        return sqrDist(pivot, a) < sqrDist(pivot, b);
//    return (order == -1);
//}

// A utility function to swap two points
void swap(Point &p1, Point &p2)
{
    Point temp = p1;
    p1 = p2;
    p2 = temp;
}

// A utility function to return square of distance
// between p1 and p2
float distSq(const Point &p1, const Point &p2)
{
    return (p1.x - p2.x)*(p1.x - p2.x) +
           (p1.y - p2.y)*(p1.y - p2.y);
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(const Point &p, const Point &q, const Point &r)
{
    float val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);

    if (feq(val, 0.0f, 0.00001f)) return 0;  // colinear
    return (val > 0)? 1: 2; // clock or counterclock wise
}

Point nextToTop(stack<Point> &S)
{
    Point p = S.top();
    S.pop();
    Point res = S.top();
    S.push(p);
    return res;
}

stack<Point> grahamScan(framebased::vector<Point> &points)    {
    stack<Point> hull;
    // Find the bottommost point
    float ymin = points[0].y;
    int min = 0;

    int n = points.size();

    for (int i = 1; i < n; i++)
    {
        float y = points[i].y;

        // Pick the bottom-most or chose the left
        // most point in case of tie
        if ((y < ymin) || (feq(ymin,y, 0.0001f) && points[i].x < points[min].x))
            ymin = points[i].y, min = i;
    }

    // Place the bottom-most point at first position
    swap(points[0], points[min]);

    // Point having the least y coordinate, used for sorting other points
    // according to polar angle about this point
    Point p0 = points[0];

    // Sort n-1 points with respect to the first point.
    // A point p1 comes before p2 in sorted ouput if p2
    // has larger polar angle (in counterclockwise
    // direction) than p1

    std::sort(points.begin()+1, points.end(), [&p0](auto const &p1, auto const &p2) -> bool {
        // Find orientation
        int o = orientation(p0, p1, p2);
        if (o == 0) {
            return feq(distSq(p0, p2), distSq(p0, p1))
                ? &p1 > &p2 // this is fallback to stabilize the sorting function
                : distSq(p0, p2) >= distSq(p0, p1);
        }

        return (o == 2);
    });

    // If two or more points make same angle with p0,
    // Remove all but the one that is farthest from p0
    // Remember that, in above sorting, our criteria was
    // to keep the farthest point at the end when more than
    // one points have same angle.
    int m = 1; // Initialize size of modified array
    for (int i=1; i<n; i++)
    {
        // Keep removing i while angle of i and i+1 is same
        // with respect to p0
        while (i < n-1 && orientation(p0, points[i],
                                      points[i+1]) == 0)
            i++;


        points[m] = points[i];
        m++;  // Update size of modified array
    }

    // If modified array of points has less than 3 points,
    // convex hull is not possible
    if (m < 3) return hull;

    hull.push(points[0]);
    hull.push(points[1]);
    hull.push(points[2]);

    for (int i = 3; i < m; i++) {
        while (hull.size() > 1 && orientation(nextToTop(hull), hull.top(), points[i]) != 2)   {
            hull.pop();
        }
        hull.push(points[i]);
    }
    return hull;
}