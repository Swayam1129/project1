#include "DrawContext.h"
#include <cmath>

namespace CMPUT350 {

/**
 * @brief Creates a drawing context that draws into the given window.
 * @param window The SFML window to draw into (shared with the game engine).
 * @param font The font used for text (shared with the game engine).
 */
DrawContext::DrawContext(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<sf::Font> font)
    : mWindow(window), mFont(font), mOffset(0, 0), mRotation(0), mCos(1), mSin(0) {}

/**
 * @brief Sets the offset added to every drawn point (the object's location).
 *        Only the GameEngine calls this, before an object draws.
 * @param p The offset in screen pixels.
 */
void DrawContext::SetContextOffset(Point2D p) { mOffset = p; }

/**
 * @brief Sets the rotation applied to every drawn point, around the object's
 *        local (0,0). cos/sin are cached here so drawing doesn't recompute them.
 * @param rotation Rotation in radians (positive is clockwise on screen).
 */
void DrawContext::SetContextRotation(float rotation) {
    mRotation = rotation;
    mCos = std::cos(rotation);
    mSin = std::sin(rotation);
}

/**
 * @brief Converts a point from the object's local space to screen space:
 *        rotate around (0,0), then add the offset.
 * @param p Point in local coordinates.
 * @return The point in screen coordinates.
 */
Point2D DrawContext::Transform(Point2D p) {
    return Point2D(p.x * mCos - p.y * mSin + mOffset.x,
                   p.x * mSin + p.y * mCos + mOffset.y);
}

/**
 * @brief Converts a point from screen space back to the object's local space:
 *        subtract the offset, then rotate by -rotation (the inverse rotation).
 * @param p Point in screen coordinates.
 * @return The point in local coordinates.
 */
Point2D DrawContext::ReverseTransform(Point2D p) {
    Point2D q = p - mOffset;
    return Point2D(q.x * mCos + q.y * mSin,
                   -q.x * mSin + q.y * mCos);
}

/**
 * @brief Transforms all four corners of a local rect to screen space and
 *        returns the axis-aligned box around them. If the object is rotated,
 *        this box is larger than the original rect.
 * @param r Rect in local coordinates.
 * @return Axis-aligned bounding box in screen coordinates.
 */
Rect DrawContext::Transform(Rect r) {
    Point2D tl = r.topLeft;
    Rect box(Transform(tl), 0, 0);
    box |= Transform(tl + Point2D(r.width, 0));
    box |= Transform(tl + Point2D(0, r.height));
    box |= Transform(tl + Point2D(r.width, r.height));
    return box;
}

/**
 * @brief Builds a 4-point SFML shape from a local rect, with each corner
 *        transformed to screen space (so it can be rotated).
 * @param r Rect in local coordinates.
 * @return Shape with the 4 transformed corners in order around the rect.
 */
sf::ConvexShape DrawContext::MakeQuad(Rect r) {
    Point2D tl = r.topLeft;
    Point2D corners[4] = {tl, tl + Point2D(r.width, 0), tl + Point2D(r.width, r.height),
                          tl + Point2D(0, r.height)};
    sf::ConvexShape shape;
    shape.setPointCount(4);
    for (int i = 0; i < 4; i++) {
        Point2D c = Transform(corners[i]);
        shape.setPoint(i, {c.x, c.y});
    }
    return shape;
}

/**
 * @brief Draws text centered on a point.
 *
 * @param text The string to draw.
 * @param pixelSize The character size in pixels.
 * @param p The center position of the text (Point2D) in pixels.
 * @param c The color of the text, specified as an RGBColor object.
 *
 * The text's origin is moved from its top-left corner to the middle of
 * its bounding box, so that positioning it at p centers it on p.
 * p is in local space; the text is rotated with the object.
 */
void DrawContext::DrawCenteredText(const std::string &text, int pixelSize, Point2D p, RGBColor c) {
    Point2D sp = Transform(p);
    sf::Text t(*mFont, text, static_cast<unsigned int>(pixelSize));
    t.setFillColor(sf::Color(c.r, c.g, c.b));
    sf::FloatRect bounds = t.getLocalBounds();
    t.setOrigin(bounds.position + bounds.size / 2.f);
    t.setPosition({sp.x, sp.y});
    t.setRotation(sf::radians(mRotation));
    mWindow->draw(t);
}

/**
 * @brief Draws text with its top-left corner at a point.
 *
 * @param text The string to draw.
 * @param pixelSize The character size in pixels.
 * @param p The top-left position of the text (Point2D) in pixels.
 * @param c The color of the text, specified as an RGBColor object.
 *
 * Uses the font shared with the game engine and renders the text onto
 * the associated window.
 * p is in local space; the text is rotated with the object.
 */
void DrawContext::DrawText(const std::string &text, int pixelSize, Point2D p, RGBColor c) {
    Point2D sp = Transform(p);
    sf::Text t(*mFont, text, static_cast<unsigned int>(pixelSize));
    t.setFillColor(sf::Color(c.r, c.g, c.b));
    t.setPosition({sp.x, sp.y});
    t.setRotation(sf::radians(mRotation));
    mWindow->draw(t);
}

/**
 * @brief Draws a filled circle centered at a point with a specified color.
 *
 * @param p The center of the circle (Point2D) in pixels.
 * @param radius The radius of the circle in pixels.
 * @param c The fill color of the circle, specified as an RGBColor object.
 *
 * SFML positions circles by the top-left corner of their bounding box
 * rather than their center, so the shape is offset by the radius in both
 * directions so that p ends up at the center. The circle is then rendered
 * onto the associated window.
 * p is in local space (rotation doesn't change a circle, only where it is).
 */
void DrawContext::DrawCircle(Point2D p, float radius, RGBColor c) {
    Point2D sp = Transform(p);
    sf::CircleShape shape(radius);
    shape.setPosition({sp.x - radius, sp.y - radius});
    shape.setFillColor(sf::Color(c.r, c.g, c.b));
    mWindow->draw(shape);
}

/**
 * @brief Draws a filled rectangle with a specified color.
 *
 * @param r The rectangle to draw (Rect), given by its top-left corner,
 *          width and height in pixels.
 * @param c The fill color of the rectangle, specified as an RGBColor object.
 *
 * The rect is in local space. Its 4 corners are transformed, so it rotates with the object.
 */
void DrawContext::DrawRect(Rect r, RGBColor c) {
    sf::ConvexShape shape = MakeQuad(r);
    shape.setFillColor(sf::Color(c.r, c.g, c.b));
    mWindow->draw(shape);
}

/**
 * @brief Draws the outline of a rectangle (no fill) with a specified
 *        thickness and color.
 *
 * @param r The rectangle to outline (Rect) in pixels.
 * @param width The thickness of the outline in pixels.
 * @param c The color of the outline, specified as an RGBColor object.
 *
 * The rect is in local space. Its 4 corners are transformed, so it rotates with the object.
 */
void DrawContext::FrameRect(Rect r, float width, RGBColor c) {
    sf::ConvexShape shape = MakeQuad(r);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineThickness(width);
    shape.setOutlineColor(sf::Color(c.r, c.g, c.b));
    mWindow->draw(shape);
}

/**
 * @brief Draws a line between two points with a specified width and color.
 *
 * @param from The starting point of the line (Point2D).
 * @param to The ending point of the line (Point2D).
 * @param width The width of the line in pixels.
 * @param c The color of the line, specified as an RGBColor object.
 *
 * SFML cannot draw thick lines directly, so the line is drawn as a
 * 4-cornered polygon (sf::ConvexShape). The direction from 'from' to 'to'
 * is normalized and rotated 90 degrees to get a sideways vector of length
 * width/2; the four corners are each endpoint offset by +/- that vector.
 * from/to are in local space and are transformed before drawing.
 */
void DrawContext::DrawLine(Point2D from, Point2D to, float width, RGBColor c) {
    from = Transform(from);
    to = Transform(to);
    Point2D dir = to - from;
    dir.Normalize();                              // unit direction of the line
    Point2D side(-dir.y, dir.x);                  // rotate 90 degrees
    side = side * (width / 2);                    // half the thickness each side

    Point2D p0 = from + side;
    Point2D p1 = to + side;
    Point2D p2 = to - side;
    Point2D p3 = from - side;

    sf::ConvexShape shape;
    shape.setPointCount(4);
    shape.setPoint(0, {p0.x, p0.y});             // corners go around the shape
    shape.setPoint(1, {p1.x, p1.y});
    shape.setPoint(2, {p2.x, p2.y});
    shape.setPoint(3, {p3.x, p3.y});
    shape.setFillColor(sf::Color(c.r, c.g, c.b));
    mWindow->draw(shape);
}

/** @return The window width in pixels. */
int DrawContext::GetWindowWidth() { return mWindow->getSize().x; }

/** @return The window height in pixels. */
int DrawContext::GetWindowHeight() { return mWindow->getSize().y; }

}  // namespace CMPUT350
