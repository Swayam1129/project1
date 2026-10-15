#ifndef GAMECONTEXT_H
#define GAMECONTEXT_H

#include <memory>

#include "DrawContext.h"
#include "EngineView.h"
#include "GameObject.h"
#include "NotificationManager.h"

namespace CMPUT350 {

/**
 * @brief Everything the engine gives an object each call: the engine view,
 *        the gameplay and GUI draw contexts, a weak pointer to the object
 *        being called, the notification manager, and the current frame number.
 */
class GameContext {
public:
    EngineView *mEngineView = nullptr;
    DrawContext *ScreenContext = nullptr;
    DrawContext *GUIContext = nullptr;
    std::weak_ptr<GameObject> CurrObject;
    NotificationManager *mNotificationManager = nullptr;
    int currentFrame = 0;
};

}  // namespace CMPUT350

#endif  // GAMECONTEXT_H