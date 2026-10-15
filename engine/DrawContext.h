#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include <cstdint>

#include "MathUtil.h"
#include <SFML/Graphics.hpp>

namespace CMPUT350 {

struct RGBColor {
    uint8_t r, g, b;
    RGBColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
};

namespace Colors {

const RGBColor red(255, 0, 0);
const RGBColor green(0, 255, 0);
const RGBColor blue(0, 0, 255);
const RGBColor yellow(255, 255, 0);
const RGBColor cyan(0, 255, 255);
const RGBColor magenta(255, 0, 255);
const RGBColor white(255, 255, 255);
const RGBColor black(0, 0, 0);
const RGBColor gray(100, 100, 100);
const RGBColor grey(200, 200, 200);

}  // namespace Colors

class DrawContext {
public:
    DrawContext(std::shared_ptr<sf::RenderWindow> mWindow, std::shared_ptr<sf::Font> font);
    void DrawText(const std::string &text, int pixelSize, Point2D p, RGBColor c);
    void DrawCenteredText(const std::string &text, int pixelSize, Point2D p, RGBColor c);
    void DrawCircle(Point2D p, float radius, RGBColor c);
    void DrawRect(Rect r, RGBColor c);
    void FrameRect(Rect r, float width, RGBColor c);
    void DrawLine(Point2D from, Point2D to, float width, RGBColor c);
    int GetWindowWidth();
    int GetWindowHeight();

private:
    friend class GameEngine;
    void SetContextOffset(Point2D p);
    void SetContextRotation(float rotation);
    Point2D Transform(Point2D p);
    Point2D ReverseTransform(Point2D p);
    Rect Transform(Rect r);  // Rotates rectangle and computes new bounding box after rotation

    sf::ConvexShape MakeQuad(Rect r);  // rect corners in screen space as a 4-point shape

    std::shared_ptr<sf::RenderWindow> mWindow;
    std::shared_ptr<sf::Font> mFont;
    Point2D mOffset;        // object location added to every point
    float mRotation;        // object rotation in radians
    float mCos, mSin;       // cached cos/sin of mRotation
};

}  // namespace CMPUT350

#endif  // RENDERTARGET_H
