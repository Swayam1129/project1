#include "GameEngine.h"
#include "DrawContext.h"
#include "GameContext.h"
#include "GraphicsObject.h"
#include "CollisionObject.h"

#include <algorithm>
#include <cmath>

namespace CMPUT350 {
#include "FontData.h"

/**
 * @brief Creates the game window, limits it to 30 FPS, loads the font
 *        from memory and creates the drawing context.
 *
 * @param width Window width in pixels.
 * @param height Window height in pixels.
 * @param name Window title.
 */
GameEngine::GameEngine(unsigned int width, unsigned int height, const std::string& name) {
    // Sample font loading code
    //	if (!mFont->openFromMemory(&_font, _font_len))
    //	{
    //		fprintf(stderr, "WARNING: Font did not load.\n");
    //	}
    mWindow = std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), name);
    mWindow->setFramerateLimit(30);

    mFont = std::make_shared<sf::Font>();
    if (!mFont->openFromMemory(_font, _font_len)) {
        fprintf(stderr, "WARNING: Font did not load.\n");
    }

    mDrawContext = std::make_unique<DrawContext>(mWindow, mFont);
    mGUIContext = std::make_unique<DrawContext>(mWindow, mFont);

    // One KeyPressed per physical press (no OS auto-repeat), so "Fire"
    // fires once per press and Move/Stop messages stay paired
    mWindow->setKeyRepeatEnabled(false);

}

/**
 * @brief Closes the window as a failsafe. Smart pointers free everything else.
 */
GameEngine::~GameEngine() {
    // Cleanup resources
    mWindow->close();
}

/**
 * @brief Queues a game object to be added to the engine at the start of
 *        the next frame (it is not added immediately).
 *
 * @param gameObject The object to add.
 */
void GameEngine::AddGameObject(std::shared_ptr<GameObject> gameObject) {
    mNewObjects.push_back(gameObject);
}

/**
 * @brief Sends a notification whenever a key is pressed. A key can have
 *        several notifications, and several keys can share one.
 * @param key SFML scancode of the key (cast to int).
 * @param notification Message sent to the NotificationManager on press.
 */
void GameEngine::InstallKeyDownNotification(int key, const std::string &notification) {
    mKeyDownNotifications[key].push_back(notification);
}

/**
 * @brief Sends a notification whenever a key is released.
 * @param key SFML scancode of the key (cast to int).
 * @param notification Message sent to the NotificationManager on release.
 */
void GameEngine::InstallKeyUpNotification(int key, const std::string &notification) {
    mKeyUpNotifications[key].push_back(notification);
}

/**
 * @brief Sends every notification registered for a key in the given table.
 * @param table mKeyDownNotifications or mKeyUpNotifications.
 * @param key SFML scancode of the key (cast to int).
 */
void GameEngine::SendKeyNotifications(const tKeyTable &table, int key) {
    auto it = table.find(key);
    if (it == table.end()) {
        return;
    }
    for (const std::string &message : it->second) {
        mNotificationManager.Notify(message);
    }
}

/** @return Const iterator to the first object in the engine. */
GameEngine::const_iterator GameEngine::cbegin() const { return mGameObjects.cbegin(); }

/** @return Const iterator past the last object in the engine. */
GameEngine::const_iterator GameEngine::cend() const { return mGameObjects.cend(); }

/** @return Same as cbegin(); allows range-based for loops over the engine. */
GameEngine::const_iterator GameEngine::begin() const { return mGameObjects.cbegin(); }

/** @return Same as cend(); allows range-based for loops over the engine. */
GameEngine::const_iterator GameEngine::end() const { return mGameObjects.cend(); }

// Narrow-phase collision helpers. All shapes here are already in screen space.

/**
 * @brief How far along a line a point lies: 0 at p1, 1 at p2.
 * @param l The line.
 * @param p A point on (or near) the line.
 * @return The projection of p onto the line, as a fraction of its length.
 */
static float LineParam(const Line &l, const Point2D &p) {
    Point2D d = l.p2 - l.p1;
    float lenSq = d * d;
    if (lenSq == 0) {
        return 0;
    }
    return ((p - l.p1) * d) / lenSq;
}

/**
 * @brief Finds where a directed line first touches a circle.
 *        Solves |p1 + t*d - c|^2 = r^2 for the smallest t in [0,1].
 * @param l The directed line (p1 -> p2).
 * @param c The circle.
 * @param t Set to the first contact position along the line (0..1).
 * @return true if the line touches the circle.
 */
static bool LineHitsCircle(const Line &l, const Circle &c, float &t) {
    Point2D d = l.p2 - l.p1;
    Point2D f = l.p1 - c.center;
    float rSq = c.radius * c.radius;
    if (f * f <= rSq) {
        t = 0;                        // starts inside the circle
        return true;
    }
    float a = d * d;
    if (a == 0) {
        return false;                 // zero-length line outside the circle
    }
    float b = 2 * (f * d);
    float disc = b * b - 4 * a * (f * f - rSq);
    if (disc < 0) {
        return false;                 // the infinite line misses the circle
    }
    t = (-b - std::sqrt(disc)) / (2 * a);   // entry point (smaller root)
    return t >= 0 && t <= 1;
}

/**
 * @brief Finds where a directed line first touches an axis-aligned rect:
 *        either its start point is inside, or the earliest crossing of an edge.
 * @param l The directed line (p1 -> p2).
 * @param r The rect.
 * @param t Set to the first contact position along the line (0..1).
 * @return true if the line touches the rect.
 */
static bool LineHitsRect(const Line &l, const Rect &r, float &t) {
    if (r.IsInside(l.p1)) {
        t = 0;
        return true;
    }
    Point2D tl = r.topLeft;
    Point2D tr = tl + Point2D(r.width, 0);
    Point2D bl = tl + Point2D(0, r.height);
    Point2D br = tl + Point2D(r.width, r.height);
    Line edges[4] = {Line(tl, tr), Line(tr, br), Line(br, bl), Line(bl, tl)};

    bool found = false;
    for (const Line &e : edges) {
        Point2D crossing;
        if (l.Crosses(e, crossing)) {
            float edgeT = LineParam(l, crossing);
            if (!found || edgeT < t) {
                t = edgeT;
                found = true;
            }
        }
    }
    return found;
}

/**
 * @brief Finds where a directed line first touches another shape.
 * @param l The directed line.
 * @param other Any shape.
 * @param t Set to the first contact position along l (0..1).
 * @return true if they touch.
 */
static bool LineFirstHit(const Line &l, const Shape &other, float &t) {
    switch (other.t) {
        case ShapeType::kLine: {
            Point2D crossing;
            if (l.Crosses(other.shape.line, crossing)) {
                t = LineParam(l, crossing);
                return true;
            }
            return false;
        }
        case ShapeType::kCircle:
            return LineHitsCircle(l, other.shape.circle, t);
        case ShapeType::kRect:
            return LineHitsRect(l, other.shape.rect, t);
    }
    return false;
}

/**
 * @brief Whether a circle and an axis-aligned rect overlap: the point of the
 *        rect closest to the circle's center must be within the radius.
 */
static bool CircleHitsRect(const Circle &c, const Rect &r) {
    Point2D closest(std::clamp(c.center.x, r.topLeft.x, r.topLeft.x + r.width),
                    std::clamp(c.center.y, r.topLeft.y, r.topLeft.y + r.height));
    return c.center.Distance(closest) <= c.radius;
}

/**
 * @brief Whether two axis-aligned rects overlap (touching edges count).
 */
static bool RectsOverlap(const Rect &a, const Rect &b) {
    return a.topLeft.x <= b.topLeft.x + b.width &&
           b.topLeft.x <= a.topLeft.x + a.width &&
           a.topLeft.y <= b.topLeft.y + b.height &&
           b.topLeft.y <= a.topLeft.y + a.height;
}

/**
 * @brief Tests two screen-space shapes. If either one is a line, also finds
 *        the first point along that line where they touch (lines are directed).
 *        Circle/rect pairs have no single contact point.
 * @param a First shape.
 * @param b Second shape.
 * @param hasPoint Set to true if a contact point was found (a line was involved).
 * @param point Set to the contact point in screen space (if hasPoint).
 * @param t Set to how far along the line the contact is, 0..1 (if hasPoint).
 * @return true if the shapes touch.
 */
static bool ShapesCollide(const Shape &a, const Shape &b, bool &hasPoint, Point2D &point, float &t) {
    hasPoint = false;
    if (a.t == ShapeType::kLine || b.t == ShapeType::kLine) {
        const Line &l = (a.t == ShapeType::kLine) ? a.shape.line : b.shape.line;
        const Shape &other = (a.t == ShapeType::kLine) ? b : a;
        if (!LineFirstHit(l, other, t)) {
            return false;
        }
        point = l.p1 + (l.p2 - l.p1) * t;
        hasPoint = true;
        return true;
    }
    if (a.t == ShapeType::kCircle && b.t == ShapeType::kCircle) {
        return a.shape.circle.center.Distance(b.shape.circle.center) <= a.shape.circle.radius + b.shape.circle.radius;
    }
    if (a.t == ShapeType::kRect && b.t == ShapeType::kRect) {
        return RectsOverlap(a.shape.rect, b.shape.rect);
    }
    if (a.t == ShapeType::kCircle) {
        return CircleHitsRect(a.shape.circle, b.shape.rect);
    }
    return CircleHitsRect(b.shape.circle, a.shape.rect);
}

/**
 * @brief Converts a shape from an object's local space to screen space using
 *        the offset/rotation currently set on the draw context. Rects become
 *        the axis-aligned box around the rotated rect.
 * @param s Shape in local space.
 * @return The same shape in screen space.
 */
Shape GameEngine::ToWorld(const Shape &s) {
    switch (s.t) {
        case ShapeType::kLine:
            return Shape(Line(mDrawContext->Transform(s.shape.line.p1), mDrawContext->Transform(s.shape.line.p2)));
        case ShapeType::kCircle:
            return Shape(Circle(mDrawContext->Transform(s.shape.circle.center), s.shape.circle.radius));
        case ShapeType::kRect:
            return Shape(mDrawContext->Transform(s.shape.rect));
    }
    return s;
}
/**
 * @brief Gives control to the game engine. Runs the game loop until the
 *        window is closed.
 *
 * Each frame: (1) remove dead objects, (2) add and initialize new objects,
 * (3) process window events and send key notifications, (4) update, (5) process collisions,
 * (6) late update, (7) render background, (8) render foreground, (9) render UI,
 * (10) display the frame.
 */
void GameEngine::Run() {
    // One context reused every frame; objects borrow these raw pointers
    GameContext context;
    context.mEngineView = this;
    context.ScreenContext = mDrawContext.get();
    context.GUIContext = mGUIContext.get();
    context.mNotificationManager = &mNotificationManager;
    context.currentFrame = 0;
    while (mWindow->isOpen()) {
        // 1. Remove dead objects: swap with last + pop_back, O(1) each, no gaps
        for (size_t i = 0; i < mGameObjects.size();) {
            if (!mGameObjects[i]->IsAlive()) {
                mGameObjects[i] = mGameObjects.back();
                mGameObjects.pop_back();   // last shared_ptr gone -> object freed
            } else {
                i++;
            }
        }
        // 2. Add new objects and initialize them
        std::vector<std::shared_ptr<GameObject>> toAdd;
        toAdd.swap(mNewObjects);
        for (auto &obj : toAdd) {
            mGameObjects.push_back(obj);
            context.CurrObject = obj;      // lets the object register itself
            obj->Initialize(&context);
        }
        
        // 3. Process SFML events
        while (const std::optional event = mWindow->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                mWindow->close();
            } else if (const auto *pressed = event->getIf<sf::Event::KeyPressed>()) {
                SendKeyNotifications(mKeyDownNotifications, static_cast<int>(pressed->scancode));
            } else if (const auto *released = event->getIf<sf::Event::KeyReleased>()) {
                SendKeyNotifications(mKeyUpNotifications, static_cast<int>(released->scancode));
            }
        }

        if (!mWindow->isOpen()) {
            break;
        }

        // 4. Update
        for (auto &obj : mGameObjects) {
            context.CurrObject = obj;
            obj->Update(&context);
        }
        
        // 5. Process collisions between collision objects
        //    Collect collision objects once per frame using RTTI
        std::vector<std::shared_ptr<CollisionObject>> colliders;
        for (auto &obj : mGameObjects) {
            auto c = std::dynamic_pointer_cast<CollisionObject>(obj);
            if (c != nullptr) {
                colliders.push_back(c);
            }
        }
        //    Convert each object's local bounds and shapes to screen space once
        //    per frame (not once per pair)
        std::vector<Rect> worldBounds;
        std::vector<std::vector<Shape>> worldShapes;
        for (auto &c : colliders) {
            mDrawContext->SetContextOffset(c->GetLocation());
            mDrawContext->SetContextRotation(c->GetRotation());
            worldBounds.push_back(mDrawContext->Transform(c->GetBounds()));
            std::vector<Shape> shapes;
            for (const Shape &s : c->GetShapes()) {
                shapes.push_back(ToWorld(s));
            }
            worldShapes.push_back(shapes);
        }

        //    Check each pair exactly once (j > i), so no self-collisions
        //    and each object gets one CollisionEnter per colliding pair
        for (size_t i = 0; i < colliders.size(); i++) {
            for (size_t j = i + 1; j < colliders.size(); j++) {
                // Broad phase: skip pairs whose bounding boxes don't overlap
                if (!RectsOverlap(worldBounds[i], worldBounds[j])) {
                    continue;
                }

                // Narrow phase: compare every shape of i with every shape of j.
                // If lines are involved keep the earliest contact along the line.
                bool hit = false;
                bool hasPoint = false;
                Point2D bestPoint;
                float bestT = 2;             // larger than any valid t (0..1)
                for (const Shape &sa : worldShapes[i]) {
                    for (const Shape &sb : worldShapes[j]) {
                        bool shapeHasPoint;
                        Point2D point;
                        float t;
                        if (ShapesCollide(sa, sb, shapeHasPoint, point, t)) {
                            hit = true;
                            if (shapeHasPoint && t < bestT) {
                                bestT = t;
                                bestPoint = point;
                                hasPoint = true;
                            }
                        }
                    }
                }
                if (!hit) {
                    continue;
                }

                if (hasPoint) {
                    // Give each object the contact point in its own local space
                    mDrawContext->SetContextOffset(colliders[i]->GetLocation());
                    mDrawContext->SetContextRotation(colliders[i]->GetRotation());
                    Point2D pointForI = mDrawContext->ReverseTransform(bestPoint);
                    mDrawContext->SetContextOffset(colliders[j]->GetLocation());
                    mDrawContext->SetContextRotation(colliders[j]->GetRotation());
                    Point2D pointForJ = mDrawContext->ReverseTransform(bestPoint);
                    colliders[i]->CollisionEnter(colliders[j], pointForI);
                    colliders[j]->CollisionEnter(colliders[i], pointForJ);
                } else {
                    colliders[i]->CollisionEnter(colliders[j]);
                    colliders[j]->CollisionEnter(colliders[i]);
                }
            }
        }

        // 6. Late updates
        for (auto &obj : mGameObjects) {
            context.CurrObject = obj;
            obj->LateUpdate(&context);
        }
        
        // Clear window
        mWindow->clear(sf::Color::Black);
        
        // 7. Render background (each object draws in its own local space)
        for (auto &obj : mGameObjects) {
            auto g = std::dynamic_pointer_cast<GraphicsObject>(obj);
            if (g != nullptr) {
                mDrawContext->SetContextOffset(g->GetLocation());
                mDrawContext->SetContextRotation(g->GetRotation());
                g->RenderBackground(&context);
            }
        }
        // 8. Render foreground
        for (auto &obj : mGameObjects) {
            auto g = std::dynamic_pointer_cast<GraphicsObject>(obj);
            if (g != nullptr) {
                mDrawContext->SetContextOffset(g->GetLocation());
                mDrawContext->SetContextRotation(g->GetRotation());
                g->RenderForeground(&context);
            }
        }
        mDrawContext->SetContextOffset(Point2D(0, 0));
        mDrawContext->SetContextRotation(0);

        // 9. Render UI on top, in plain screen coordinates (GUI context is never transformed)
        for (auto &obj : mGameObjects) {
            obj->RenderUI(&context);
        }

        // 10. Display the frame
        mWindow->display();
        context.currentFrame++;            // global clock for all objects
    }
}

}  // namespace CMPUT350
