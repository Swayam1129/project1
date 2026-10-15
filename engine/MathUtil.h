#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <cmath>
#include <iostream>
#include <algorithm>

namespace CMPUT350 {

/** @brief A 2D point or vector in pixel coordinates. */
struct Point2D {
    float x, y;
    /** @brief Creates a point (defaults to (0,0)). */
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}

    /**
     * @brief Straight-line distance between this point and another.
     * @param other The other point.
     * @return The distance in pixels.
     */
    double Distance(const Point2D &other) const {
        Point2D d = *this - other;
        return std::sqrt(d.x*d.x + d.y*d.y);
    }

    /**
     * @brief Adds another point to this one, component by component.
     * @param other The point to add.
     * @return A new point (this->x + other.x, this->y + other.y); this point is unchanged.
     */
    Point2D operator+(const Point2D &other) const {
        return Point2D(this->x + other.x, this->y + other.y);
    }

    /**
     * @brief Adds the same number to both x and y.
     * @param other The number to add.
     * @return A new point (this->x + other , this->y + other); this point is unchanged.
     */
    Point2D operator+(const float &other) const {
        return Point2D(this->x + other , this->y + other);
    }

    /**
     * @brief Subtracts another point from this one, component by component.
     *        The result is the vector pointing from other to this point.
     * @param other The point to subtract.
     * @return A new point (this->x - other.x, this->y - other.y); this point is unchanged.
     */
    Point2D operator-(const Point2D &other) const {
        return Point2D(this->x - other.x, this->y - other.y);
    }

    /**
     * @brief Subtracts the same number from both x and y.
     * @param other The number to subtract.
     * @return A new point (this->x - other , this->y - other); this point is unchanged.
     */
    Point2D operator-(const float &other) const {
        return Point2D(this->x - other , this->y - other);
    }

    /**
     * @brief Scales both x and y by a number (stretches or shrinks the vector).
     * @param scalar The number to multiply by.
     * @return A new point (this->x*scalar, this->y*scalar); this point is unchanged.
     */
    Point2D operator*(const float &scalar) const {
        return Point2D(this->x*scalar, this->y*scalar);
    }

    /**
     * @brief Adds the same number to both x and y, in place.
     * @param scalar The number to add.
     * @return A reference to this (modified) point.
     */
    Point2D &operator+=(const float &scalar) {
        this->x += scalar;
        this->y += scalar;
        return *this;
    }

    /**
     * @brief Adds another point to this one, in place (e.g. x += y).
     * @param other The point to add.
     * @return A reference to this (modified) point.
     */
    Point2D &operator+=(const Point2D &other) {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }

    /**
     * @brief Subtracts another point from this one, in place. (e.g. x -= y).
     * @param other The point to subtract.
     * @return A reference to this (modified) point.
     */
    Point2D &operator-=(const Point2D &other) {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }

    /**
     * @brief Checks whether two points have the same coordinates.
     * @param other The point to compare with.
     * @return true if both x and y are equal, false otherwise.
     */
    bool operator==(const Point2D &other) const {
        return (this->x == other.x && this->y == other.y);
    }

    /**
     * @brief Multiplies both x and y by a whole number, in place.
     * @param scalar The integer to multiply by.
     * @return A reference to this (modified) point.
     */
    Point2D &operator*=(const int &scalar) {
        this->x *= scalar;
        this->y *= scalar;
        return *this;
    }

    /**
     * @brief Divides both x and y by a whole number, in place.
     *        Dividing by 0 would produce inf/NaN, so in that case the
     *        point is left unchanged.
     * @param scalar The integer to divide by (int, as given in the spec).
     * @return A reference to this point.
     */
    Point2D &operator/=(const int &scalar) {
        if (scalar == 0){
            return *this;
        }
        this->x /= scalar;
        this->y /= scalar;
        return *this;
    }

    /**
     * @brief Dot product of this point and another: x*other.x + y*other.y.
     * @param other The other point (vector).
     * @return A single number: positive if the vectors point roughly the same
     *         way, 0 if perpendicular, negative if opposite.
     */
    float operator*(const Point2D &other) const {
        float px = this->x * other.x;
        float py = this->y * other.y;
        return  px+py;
    }

    /**
     * @brief Dot product of this point with b .
     * @param b The other point (vector).
     * @return The dot product.
     */
    float Dot(Point2D b) const {
        return *this *b;
    }

    /**
     * @brief Dot product of two points. Static, so it is called as
     *        Point2D::Dot(a, b) rather than on a point.
     * @param a The first point (vector).
     * @param b The second point (vector).
     * @return a.x*b.x + a.y*b.y
     */
    static float Dot(Point2D a, Point2D b) {
        return a*b;
    }

    /**
     * @brief 2D cross product of this point with b (same as Cross(*this, b)).
     * @param b The other point (vector).
     * @return x*b.y - y*b.x; its sign tells which side of this vector b is on.
     */
    float Cross(Point2D b) const {
       return Cross(*this, b);   
    }

    /**
     * @brief 2D "cross product" of two vectors: a.x*b.y - a.y*b.x.
     * @param a The first point (vector).
     * @param b The second point (vector).
     * @return Positive if b is on one side of a, negative if on the other,
     *         0 if they lie on the same line.
     */
    static float Cross(Point2D a, Point2D b) {
        return a.x*b.y - a.y*b.x;
    }

    /**
     * @brief Scales this point to length 1, keeping its direction.
     *        The length is measured once before changing x and y. A zero
     *        vector has no direction, so it is left unchanged.
     */
    void Normalize() {
        Point2D orig(0,0);
        float len= this->Distance(orig);
        if (len ==0 ){
            return;
        }
        this->x /= len;
        this->y /= len;
        return;
    }
};

/**
 * @brief Prints a point as (x,y).
 * @param os The output stream to write to.
 * @param p The point to print.
 * @return The same stream, so calls can be chained.
 */
static std::ostream &operator<<(std::ostream &os, const Point2D &p) {
    os << "(" << p.x << "," << p.y << ")" ;
    return os;
}

/**
 * @brief Allows a number on the left side.
 * @param number The number to multiply by.
 * @param rhs The point to scale.
 * @return A new scaled point.
 */
static Point2D operator*(float number, const Point2D &rhs) {
    Point2D multiple = rhs * number;
    return multiple;
}

/** @brief A line segment from p1 to p2. Treated as directed (p1 -> p2) for collisions. */
struct Line {
    Point2D p1, p2;

    /** @brief Creates a line from two points (defaults to a zero-length line at (0,0)). */
    Line(Point2D p1 = {0, 0}, Point2D p2 = {0, 0}) : p1(p1), p2(p2) {}

    /** @brief Creates a line from the coordinates of its two endpoints. */
    Line(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}

    /**
     * @brief Length of the segment.
     * @return Distance from p1 to p2 in pixels.
     */
    float Length() const {
        return static_cast<float>(p1.Distance(p2));
    }

    /**
     * @brief Finds the point on this segment closest to p.
     *        p is projected onto the line, and the projection is clamped
     *        to the segment so the result never lies past p1 or p2.
     * @param p The point to measure from.
     * @return The closest point on the segment.
     */
    Point2D ClosestPoint(const Point2D &p) const {
        Point2D d = p2 - p1;
        float lenSq = d * d;              // squared length (dot product with itself)
        if (lenSq == 0) {
            return p1;                    // segment is a single point
        }
        float t = ((p - p1) * d) / lenSq; // how far along the segment (0 = p1, 1 = p2)
        t = std::clamp(t, 0.0f, 1.0f);    // stay on the segment
        return p1 + d * t;
    }

    /**
     * @brief Checks whether this segment crosses another segment.
     * @param other The other segment.
     * @param crossingPoint Set to the intersection point if they cross.
     * @return true if the segments intersect; false if they don't or are parallel.
     */
    bool Crosses(const Line &other, Point2D &crossingPoint) const {
        Point2D r = p2 - p1;              // direction of this line
        Point2D s = other.p2 - other.p1;  // direction of the other line
        float denom = Point2D::Cross(r, s);
        if (denom == 0) {
            return false;                 // parallel (or collinear): no single crossing point
        }
        Point2D q = other.p1 - p1;
        float t = Point2D::Cross(q, s) / denom;  // position along this line
        float u = Point2D::Cross(q, r) / denom;  // position along the other line
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            crossingPoint = p1 + r * t;
            return true;
        }
        return false;
    }
};

/**
 * @brief Prints a line as (x1,y1)->(x2,y2).
 * @param os The output stream to write to.
 * @param l The line to print.
 * @return The same stream, so calls can be chained.
 */
static std::ostream &operator<<(std::ostream &os, const Line &l) {
    os << l.p1 << "->" << l.p2;
    return os;
}

/** @brief A circle given by its center and radius. */
struct Circle {
    Point2D center;
    float radius;

    Circle(Point2D c = {0, 0}, float r = 0) : center(c), radius(r) {}

    Circle(float x, float y, float r) : center(x, y), radius(r) {}
};

/**
 * @brief Prints a circle as [center:(x,y) r:radius].
 * @param os The output stream to write to.
 * @param c The circle to print.
 * @return The same stream, so calls can be chained.
 */
static std::ostream &operator<<(std::ostream &os, const Circle &c) {
    os << "[center:" << c.center << " r:" << c.radius << "]";
    return os;
}

/** @brief An axis-aligned rectangle: top-left corner plus width and height. */
struct Rect {
    Point2D topLeft;
    float width, height;

    /**
     * @brief Creates a rect from its left, top, width and height.
     */
    Rect(float left, float top, float width, float height)
        : topLeft(Point2D(left, top)), width(width), height(height) {}

    /** @brief Creates a rect from its top-left corner, width and height. */
    Rect(Point2D tl = {0, 0}, int w = 0, int h = 0) : topLeft(tl), width(w), height(h) {}

    // Creates bounding box around p1 and p2 with positive width/height
    Rect(Point2D p1, Point2D p2)
        : topLeft(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
          width(fabs(p1.x - p2.x)),
          height(fabs(p1.y - p2.y)) {}

    /** @brief Creates the square box around a circle with the given center and radius. */
    Rect(Point2D center, float radius)
        : topLeft(center.x - radius, center.y - radius), width(2 * radius), height(2 * radius) {}
    
    /**
     * @brief Grows this rect to the smallest box containing both rects (union).
     *        All edges are computed before any member is changed.
     * @param other The rect to include.
     * @return A reference to this rect.
     */
    Rect &operator|=(const Rect &other) {
        float left   = std::min(topLeft.x, other.topLeft.x);
        float top    = std::min(topLeft.y, other.topLeft.y);
        float right  = std::max(topLeft.x + width,  other.topLeft.x + other.width);
        float bottom = std::max(topLeft.y + height, other.topLeft.y + other.height);

        topLeft = Point2D(left, top);
        width   = right - left;
        height  = bottom - top;
        return *this;
    }

    /**
     * @brief Grows this rect to include a point (treated as a zero-size rect).
     * @param other The point to include.
     * @return A reference to this rect.
     */
    Rect &operator|=(const Point2D &other) {
        return *this |= Rect(other, 0, 0);
    }


    /**
     * @brief Grows this rect to include both endpoints of a line.
     *        A straight segment never leaves the box around its endpoints,
     *        so including both endpoints covers the whole line.
     * @param other The line to include.
     * @return A reference to this rect.
     */
    Rect &operator|=(const Line &other) {
        *this |= other.p1;
        *this |= other.p2;
        return *this;
    }

    /**
     * @brief Shrinks this rect to the area it shares with another (intersection).
     *        If they don't overlap, width and height become 0.
     * @param other The rect to intersect with.
     * @return A reference to this rect.
     */
    Rect &operator&=(const Rect &other) {
        float left   = std::max(topLeft.x, other.topLeft.x);
        float top    = std::max(topLeft.y, other.topLeft.y);
        float right  = std::min(topLeft.x + width,  other.topLeft.x + other.width);
        float bottom = std::min(topLeft.y + height, other.topLeft.y + other.height);

        topLeft = Point2D(left, top);
        // If the boxes don't overlap, right < left (or bottom < top), so clamp to 0
        width  = std::max(0.0f, right - left);
        height = std::max(0.0f, bottom - top);
        return *this;
    }

    /**
     * @brief Moves this rect by an offset; its size stays the same.
     * @param other The offset to move by.
     * @return A reference to this rect.
     */
    Rect &operator+=(const Point2D &other) {
        topLeft += other;
        return *this;
    }

    /**
     * @brief Returns a copy of this rect moved by an offset.
     * @param other The offset to move by.
     * @return A new moved rect; this rect is unchanged.
     */
    Rect operator+(const Point2D &other) const {
        Rect result = *this;
        result += other;
        return result;
    }

    /**
     * @brief Moves every edge inward by inset pixels (a negative inset grows
     *        the rect). Width and height never go below 0.
     * @param inset Pixels to move each edge inward.
     */
    void Inset(int inset) {
        topLeft.x += inset;
        topLeft.y += inset;
        width  = std::max(0.0f, width  - 2 * inset);
        height = std::max(0.0f, height - 2 * inset);

    }

    /**
     * @brief Checks whether a point lies inside this rect (edges count as inside).
     * @param p The point to test.
     * @return true if p is inside or on the edge, false otherwise.
     */
    bool IsInside(const Point2D &p) const {
        return p.x >= topLeft.x && p.x <= topLeft.x + width &&
           p.y >= topLeft.y && p.y <= topLeft.y + height;
    }
};

/**
 * @brief Prints a rect as [(x,y) w:width h:height].
 * @param os The output stream to write to.
 * @param l The rect to print.
 * @return The same stream, so calls can be chained.
 */
static std::ostream &operator<<(std::ostream &os, const Rect &l) {
    os << "[" << l.topLeft << " w:" << l.width << " h:" << l.height << "]";
    return os;
}

}  // namespace CMPUT350

#endif  // MATHUTIL_H
