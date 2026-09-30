#include "DrawContext.h"

namespace CMPUT350 {

/**
 * @brief Creates a drawing context that draws into the given window.
 * @param window The SFML window to draw into (shared with the game engine).
 * @param font The font used for text (shared with the game engine).
 */
DrawContext::DrawContext(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<sf::Font> font)
    : mWindow(window), mFont(font) {}

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
 */
void DrawContext::DrawCenteredText(const std::string &text, int pixelSize, Point2D p, RGBColor c) {
    sf::Text t(*mFont, text, static_cast<unsigned int>(pixelSize));
    t.setFillColor(sf::Color(c.r, c.g, c.b));
    sf::FloatRect bounds = t.getLocalBounds();
    t.setOrigin(bounds.position + bounds.size / 2.f);
    t.setPosition({p.x, p.y});
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
 */
void DrawContext::DrawText(const std::string &text, int pixelSize, Point2D p, RGBColor c) {
    sf::Text t(*mFont, text, static_cast<unsigned int>(pixelSize));
    t.setFillColor(sf::Color(c.r, c.g, c.b));
    t.setPosition({p.x, p.y});
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
 */
void DrawContext::DrawCircle(Point2D p, float radius, RGBColor c) {
    sf::CircleShape shape(radius);
    shape.setPosition({p.x - radius, p.y - radius});
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
 * This function creates an SFML rectangle shape of the rect's size,
 * positions it at the rect's top-left corner, fills it with the given
 * color and renders it onto the associated window.
 */
void DrawContext::DrawRect(Rect r, RGBColor c) {
    sf::RectangleShape shape({r.width, r.height});
    shape.setPosition({r.topLeft.x, r.topLeft.y});
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
 * This function creates an SFML rectangle shape with a transparent fill
 * and a colored outline. It is useful for debugging, e.g. drawing an
 * object's collision bounds on top of it.
 */
void DrawContext::FrameRect(Rect r, float width, RGBColor c) {

    sf::RectangleShape shape({r.width, r.height});
    shape.setPosition({r.topLeft.x, r.topLeft.y});
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
 */
void DrawContext::DrawLine(Point2D from, Point2D to, float width, RGBColor c) {
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
