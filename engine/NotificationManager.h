#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace CMPUT350 {

class GameObject;

/**
 * @brief Passes named messages to every object registered for that name,
 *        so objects can talk without holding references to each other.
 *        Listeners are weak pointers, so the manager never keeps an object alive.
 */
class NotificationManager {
public:
    void Register(std::weak_ptr<GameObject> object, const std::string &key);
    void Unregister(std::weak_ptr<GameObject> object, const std::string &key);
    void Notify(const std::string &message);

private:
    std::unordered_map<std::string, std::vector<std::weak_ptr<GameObject>>> mListeners;
};

}  // namespace CMPUT350

#endif  // NOTIFICATIONMANAGER_H