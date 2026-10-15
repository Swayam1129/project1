#include "GameObject.h"

namespace CMPUT350 {

// Default implementations: they do nothing, so a derived game object only
// overrides the functions it needs.

/**
 * @brief Called once when the object is added to the engine.
 * @param context Game context.
 */
void GameObject::Initialize(GameContext *context) { return; }

/**
 * @brief Called every frame for game logic (step 4 of game loop).
 * @param context Game context.
 */
void GameObject::Update(GameContext *context) { return; }

/**
 * @brief Called every frame after collisions (step 6 of game loop).
 * @param context Game context.
 */
void GameObject::LateUpdate(GameContext *context) { return; }

/**
 * @brief Draws UI on top of the game (not used in 1a).
 * @param contextrender Game context.
 */
void GameObject::RenderUI(GameContext *contextrender) { return; }

/**
 * @brief Whether the object should stay in the engine.
 * @return true by default; dead objects are removed at the start of the next frame.
 */
bool GameObject::IsAlive() const { return true; }

/**
 * @brief Marks the object as dead. Does nothing by default.
 */
void GameObject::Kill() {}

/**
 * @brief Called by the NotificationManager when a message this object
 *        registered for is sent. Does nothing by default.
 * @param key The message name.
 */
void GameObject::ReceiveNotification(const std::string &key) {}
}  // namespace CMPUT350
