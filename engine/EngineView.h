#ifndef ENGINEVIEW_H
#define ENGINEVIEW_H

#include <memory>
#include <string>
#include <vector>

namespace CMPUT350 {

class GameObject;

/**
 * @brief The part of the engine that game objects are allowed to use:
 *        adding objects, registering key notifications, and iterating
 *        over the objects currently in the engine.
 */
class EngineView {
    using tGameObject = std::vector<std::shared_ptr<GameObject>>;

public:
    virtual ~EngineView() = default;
    virtual void AddGameObject(std::shared_ptr<GameObject> gameObject) = 0;
    virtual void InstallKeyDownNotification(int key, const std::string &notification) = 0;
    virtual void InstallKeyUpNotification(int key, const std::string &notification) = 0;
    using const_iterator = tGameObject::const_iterator;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator cend() const = 0;
    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
};

}  // namespace CMPUT350

#endif