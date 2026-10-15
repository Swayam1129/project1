#include "Bezier.h"

#include <algorithm>
#include <cmath>

namespace CMPUT350 {

/**
 * @brief Creates a Bezier curve from its control points.
 * @param pts Control points; should be k*3+1 points long (k >= 1).
 */
Bezier::Bezier(const std::vector<Point2D> &pts) : mPts(pts) {}

/**
 * @brief Point on the curve at time t (0 <= t <= GetMaxT()).
 * @param t Curve time; whole part picks the segment, fraction is position in it.
 * @return The point on the curve.
 */
Point2D Bezier::GetPoint(float t) const { return GetPoint(mPts, t); }

/**
 * @brief Direction of travel (derivative) at time t.
 * @param t Curve time.
 * @return The slope vector (not normalized).
 */
Point2D Bezier::GetSlope(float t) const { return GetSlope(mPts, t); }

/**
 * @brief Largest valid t for this curve (the number of segments).
 * @return Number of cubic segments.
 */
float Bezier::GetMaxT() const { return GetMaxT(mPts); }

/**
 * @brief Number of full cubic segments in a list of control points.
 * @param pts Control points.
 * @return (pts.size() - 1) / 3, or 0 if there are fewer than 4 points.
 */
float Bezier::GetMaxT(const std::vector<Point2D> &pts) {
    if (pts.size() < 4) {
        return 0;
    }
    return static_cast<float>((pts.size() - 1) / 3);
}

/**
 * @brief Splits t into a segment index and a local u in [0,1].
 *        t is clamped to [0, max t]. t == max t uses the end of the last segment.
 * @param pts Control points.
 * @param t Curve time.
 * @param u Set to the position inside the chosen segment (0..1).
 * @return Index of the segment's first control point (segment * 3).
 */
int Bezier::GetSegment(const std::vector<Point2D> &pts, float t, float &u) {
    int numSegments = static_cast<int>(GetMaxT(pts));
    t = std::clamp(t, 0.0f, static_cast<float>(numSegments));
    int seg = static_cast<int>(std::floor(t));
    if (seg >= numSegments) {
        seg = numSegments - 1;     // t at the very end: last segment, u = 1
    }
    u = t - seg;
    return seg * 3;
}

/**
 * @brief Point on a chained cubic Bezier curve at time t.
 * @param pts Control points (k*3+1 of them).
 * @param t Curve time, 0 <= t <= k.
 * @return The point on the curve, or the first point if there are too few points.
 */
Point2D Bezier::GetPoint(const std::vector<Point2D> &pts, float t) {
    if (pts.size() < 4) {
        return pts.empty() ? Point2D() : pts[0];
    }
    float u;
    int i = GetSegment(pts, t, u);
    float v = 1 - u;
    return v * v * v * pts[i]
         + 3 * v * v * u * pts[i + 1]
         + 3 * v * u * u * pts[i + 2]
         + u * u * u * pts[i + 3];
}

/**
 * @brief Slope (derivative) of a chained cubic Bezier curve at time t.
 * @param pts Control points (k*3+1 of them).
 * @param t Curve time, 0 <= t <= k.
 * @return The slope vector (not normalized), or (0,0) if there are too few points.
 */
Point2D Bezier::GetSlope(const std::vector<Point2D> &pts, float t) {
    if (pts.size() < 4) {
        return Point2D();
    }
    float u;
    int i = GetSegment(pts, t, u);
    float v = 1 - u;
    return 3 * v * v * (pts[i + 1] - pts[i])
         + 6 * v * u * (pts[i + 2] - pts[i + 1])
         + 3 * u * u * (pts[i + 3] - pts[i + 2]);
}

}  // namespace CMPUT350
