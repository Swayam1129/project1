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
 * @brief Gives control to the game engine. Runs the game loop until the
 *        window is closed.
 *
 * Each frame: (1) remove dead objects, (2) add and initialize new objects,
 * (3) process window/keyboard events, (4) update, (5) process collisions,
 * (6) late update, (7) render background, (8) render foreground,
 * (9) display the frame.
 */
void GameEngine::Run() {
    // One context reused every frame; objects borrow these raw pointers
    GameContext context;
    context.mEngineView = this;
    context.ScreenContext = mDrawContext.get();
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
            obj->Initialize(&context);
        }
        
        // 3. Process SFML events
        while (const std::optional event = mWindow->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                mWindow->close();
            } else if (const auto *keyPressed = event->getIf<sf::Event::TextEntered>()) {
                if (keyPressed->unicode < 128) {   // low-order ASCII only
                    char key = static_cast<char>(keyPressed->unicode);
                    for (auto &obj : mGameObjects) {
                        obj->HandleKeyEvent(&context, key);
                    }
                }
            }
        }

        if (!mWindow->isOpen()) {
            break;
        }

        // 4. Update
        for (auto &obj : mGameObjects) {
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
        //    Check each pair exactly once (j > i), so no self-collisions
        //    and each object gets one CollisionEnter per colliding pair
        for (size_t i = 0; i < colliders.size(); i++) {
            for (size_t j = i + 1; j < colliders.size(); j++) {
                // Copy the bounds: GetBounds() may return a reference to a
                // shared static Rect, which the next call would overwrite
                Rect a = colliders[i]->GetBounds();
                Rect b = colliders[j]->GetBounds();

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
            obj->LateUpdate(&context);
        }
        
        // Clear window
        mWindow->clear(sf::Color::Black);
        
        // 7. Render background
        for (auto &obj : mGameObjects) {
            auto g = std::dynamic_pointer_cast<GraphicsObject>(obj);
            if (g != nullptr) {
                g->RenderBackground(&context);
            }
        }
        //8. Render foreground
        for (auto &obj : mGameObjects) {
            auto g = std::dynamic_pointer_cast<GraphicsObject>(obj);
            if (g != nullptr) {
                g->RenderForeground(&context);
            }
        }
        // 9. Display the frame
        mWindow->display();
    }
}

}  // namespace CMPUT350
