#ifndef GAMEENGINE_H
#define GAMEENGINE_H

namespace CMPUT350 {
class GameEngine;
}

#include "EngineView.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "CollisionObject.h"
#include "NotificationManager.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace CMPUT350 {

class DrawContext;

class GameEngine : public EngineView {
public:
    GameEngine(unsigned int width, unsigned int height, const std::string& name);
    ~GameEngine();

    GameEngine(const GameEngine&) = delete;             // Prevent copy-construction
    GameEngine(GameEngine&&) = delete;                  // Prevent move-construction
    GameEngine& operator=(const GameEngine&) = delete;  // Prevent assignment
    GameEngine& operator=(GameEngine&&) = delete;       // Prevent move-assignment

    void AddGameObject(std::shared_ptr<GameObject> gameObject) override;
    void InstallKeyDownNotification(int key, const std::string &notification) override;
    void InstallKeyUpNotification(int key, const std::string &notification) override;
    const_iterator cbegin() const override;
    const_iterator cend() const override;
    const_iterator begin() const override;
    const_iterator end() const override;

    void Run();

private:
    using tKeyTable = std::unordered_map<int, std::vector<std::string>>;

    void SendKeyNotifications(const tKeyTable &table, int key);
    Shape ToWorld(const Shape &s);   // local shape -> screen space (uses current DrawContext transform)

    std::shared_ptr<sf::RenderWindow> mWindow;     // shared with DrawContext
    std::shared_ptr<sf::Font> mFont;               // shared with DrawContext
    std::unique_ptr<DrawContext> mDrawContext;     // gameplay drawing
    std::unique_ptr<DrawContext> mGUIContext;      // GUI drawing (no object transforms)
    std::vector<std::shared_ptr<GameObject>> mGameObjects;  // objects in play
    std::vector<std::shared_ptr<GameObject>> mNewObjects;   // added next frame
    NotificationManager mNotificationManager;      // owned by the engine
    tKeyTable mKeyDownNotifications;               // scancode -> messages on press
    tKeyTable mKeyUpNotifications;                 // scancode -> messages on release
};

}  // namespace CMPUT350

#endif  // GAMEENGINE_H
