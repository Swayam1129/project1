#include "GameEngine.h"
#include "DrawContext.h"
#include "GameContext.h"
#include "GraphicsObject.h"
#include "CollisionObject.h"


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


        // 5. Process collisions between collision objects (AABB overlap)
        //    Collect collision objects once per frame using RTTI
        std::vector<std::shared_ptr<CollisionObject>> colliders;
        for (auto &obj : mGameObjects) {
            auto c = std::dynamic_pointer_cast<CollisionObject>(obj);
            if (c != nullptr) {
                colliders.push_back(c);
            }
        }
        //    Each object's local bounds -> screen-space box, computed once per
        //    object per frame (not once per pair)
        std::vector<Rect> worldBounds;
        for (auto &c : colliders) {
            mDrawContext->SetContextOffset(c->GetLocation());
            mDrawContext->SetContextRotation(c->GetRotation());
            worldBounds.push_back(mDrawContext->Transform(c->GetBounds()));
        }

        //    Check each pair exactly once (j > i), so no self-collisions
        //    and each object gets one CollisionEnter per colliding pair
        for (size_t i = 0; i < colliders.size(); i++) {
            for (size_t j = i + 1; j < colliders.size(); j++) {
                const Rect &a = worldBounds[i];
                const Rect &b = worldBounds[j];

                // Boxes overlap if they overlap on both axes.
                // <= so zero-width boxes (e.g. a bullet moving straight up) still collide
                bool overlap = a.topLeft.x <= b.topLeft.x + b.width &&
                               b.topLeft.x <= a.topLeft.x + a.width &&
                               a.topLeft.y <= b.topLeft.y + b.height &&
                               b.topLeft.y <= a.topLeft.y + a.height;
                if (overlap) {
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
