#ifndef BEZIER_H
#define BEZIER_H

#include <vector>

#include "MathUtil.h"

namespace CMPUT350 {

/**
 * @brief Chained cubic Bezier curves. Every 3 points after the first add one
 *        4-point segment (segments share endpoints). With k*3+1 points,
 *        t ranges from 0 to k.
 */
class Bezier {
public:
    Bezier(const std::vector<Point2D> &pts);

    Point2D GetPoint(float t) const;
    Point2D GetSlope(float t) const;
    float GetMaxT() const;

    static Point2D GetPoint(const std::vector<Point2D> &pts, float t);
    static Point2D GetSlope(const std::vector<Point2D> &pts, float t);
    static float GetMaxT(const std::vector<Point2D> &pts);

private:
    std::vector<Point2D> mPts;   // control points

    static int GetSegment(const std::vector<Point2D> &pts, float t, float &u);
};

}  // namespace CMPUT350

#endif  // BEZIER_H
