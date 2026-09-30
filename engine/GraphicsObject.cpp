#include "GraphicsObject.h"

namespace CMPUT350 {

/**
 * @brief Draws the object's background layer. All backgrounds are drawn
 *        before any foregrounds. Does nothing by default.
 * @param contextrender Game context.
 */
void GraphicsObject::RenderBackground(GameContext *contextrender) { return; }

/**
 * @brief Draws the object's foreground layer. Does nothing by default.
 * @param contextrender Game context.
 */
void GraphicsObject::RenderForeground(GameContext *contextrender) { return; }

}  // namespace CMPUT350
