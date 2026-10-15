#include "NotificationManager.h"

#include <algorithm>

#include "GameObject.h"

namespace CMPUT350 {

/**
 * @brief True if two weak pointers refer to the same object. Uses owner_before,
 *        which still works after the object has been destroyed.
 */
static bool SameObject(const std::weak_ptr<GameObject> &a, const std::weak_ptr<GameObject> &b) {
    return !a.owner_before(b) && !b.owner_before(a);
}

/**
 * @brief Registers an object to receive a message. Registering the same
 *        object twice for the same message has no extra effect.
 * @param object The object that wants the message (held weakly).
 * @param key The message name, e.g. "MoveLeft".
 */
void NotificationManager::Register(std::weak_ptr<GameObject> object, const std::string &key) {
    auto &list = mListeners[key];
    for (auto &existing : list) {
        if (SameObject(existing, object)) {
            return;
        }
    }
    list.push_back(object);
}

/**
 * @brief Stops an object from receiving a message.
 * @param object The object to remove.
 * @param key The message name.
 */
void NotificationManager::Unregister(std::weak_ptr<GameObject> object, const std::string &key) {
    auto it = mListeners.find(key);
    if (it == mListeners.end()) {
        return;
    }
    auto &list = it->second;
    list.erase(std::remove_if(list.begin(), list.end(),
                              [&](const std::weak_ptr<GameObject> &w) { return SameObject(w, object); }),
               list.end());
}

/**
 * @brief Sends a message to every live object registered for it, immediately.
 *        Listeners that were destroyed or are no longer alive are removed.
 * @param message The message name.
 */
void NotificationManager::Notify(const std::string &message) {
    auto it = mListeners.find(message);
    if (it == mListeners.end()) {
        return;
    }
    auto &list = it->second;

    // Drop listeners that are gone or dead
    list.erase(std::remove_if(list.begin(), list.end(),
                              [](const std::weak_ptr<GameObject> &w) {
                                  auto obj = w.lock();
                                  return obj == nullptr || !obj->IsAlive();
                              }),
               list.end());

    // Copy first: a listener may Register/Unregister while handling the
    // message, which would invalidate iterators into 'list'
    std::vector<std::shared_ptr<GameObject>> targets;
    for (auto &w : list) {
        targets.push_back(w.lock());
    }
    for (auto &obj : targets) {
        obj->ReceiveNotification(message);
    }
}

}  // namespace CMPUT350