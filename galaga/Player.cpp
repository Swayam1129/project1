#include <algorithm>
#include <cassert>
#include "Player.h"
#include "Bullet.h"

static const float kPlayerSize = 40.0f;     // ship is about 40x40
static const float kMoveStep = 10.0f;       // pixels per key press
static const float kBulletSpeed = 15.0f;    // pixels per frame

/**
 * @brief Creates the player ship.
 * @param loc The center of the ship in pixels.
 */
Player::Player(CMPUT350::Point2D loc): mLoc(loc), mAlive(true)
{
    UpdateBounds();
}

/**
 * @brief Rebuilds the 40x40 collision box centered on the ship's position.
 *        Called whenever the ship moves.
 */
void Player::UpdateBounds()
{
    mBounds = CMPUT350::Rect(mLoc.x - kPlayerSize / 2, mLoc.y - kPlayerSize / 2,
                             kPlayerSize, kPlayerSize);
}

/**
 * @brief Nothing to set up when the player is added to the engine.
 * @param context Game context.
 */
void Player::Initialize(CMPUT350::GameContext* context)
{
}

/**
 * @brief The player only moves in response to keys, so there is nothing to
 *        update each frame.
 * @param context Game context.
 */
void Player::Update(CMPUT350::GameContext* context)
{
}

/**
 * @brief No late update needed for the player.
 * @param context Game context.
 */
void Player::LateUpdate(CMPUT350::GameContext* context)
{
}

/**
 * @brief Moves the ship with A/D and fires with space.
 *
 * At most two player bullets can exist at once. Bullets are tracked with
 * weak_ptrs, which don't keep them alive: once the engine removes a bullet,
 * its weak_ptr expires and that slot can fire again.
 *
 * @param context The game context (used for the screen width and to add bullets).
 * @param key The key that was pressed.
 * @return true if the key was used by the player.
 */
bool Player::HandleKeyEvent(CMPUT350::GameContext* context, char key)
{
    float half = kPlayerSize / 2;
    float maxX = context->ScreenContext->GetWindowWidth() - half;

    if (key == 'a' || key == 'A')
    {
        mLoc.x = std::max(half, mLoc.x - kMoveStep);   // don't leave the screen
        UpdateBounds();
        return true;
    }
    if (key == 'd' || key == 'D')
    {
        mLoc.x = std::min(maxX, mLoc.x + kMoveStep);
        UpdateBounds();
        return true;
    }
    if (key == ' ')
    {
        for (auto &slot : mBullets)
        {
            if (slot.expired())   // this bullet is gone (or never fired)
            {
                auto bullet = std::make_shared<Bullet>(
                    CMPUT350::Point2D(mLoc.x, mLoc.y - half - 1),   // just above the ship
                    CMPUT350::Point2D(0, -kBulletSpeed),             // straight up
                    true);                                            // player bullet
                context->mEngineView->AddGameObject(bullet);          // engine owns it
                slot = bullet;                                        // we only watch it
                break;                                                // one bullet per press
            }
        }
        return true;
    }
    return false;
}

/**
 * @brief The player has no background layer.
 * @param context Game context.
 */
void Player::RenderBackground(CMPUT350::GameContext* context)
{
}

/**
 * @brief Draws the ship from several shapes centered on its position.
 * @param context Game context (used for its DrawContext).
 */
void Player::RenderForeground(CMPUT350::GameContext* context)
{
    CMPUT350::DrawContext *d = context->ScreenContext;
    float x = mLoc.x, y = mLoc.y;

    d->DrawRect(CMPUT350::Rect(x - 20, y + 2, 40, 10), CMPUT350::Colors::gray);   // wings
    d->DrawRect(CMPUT350::Rect(x - 6, y - 12, 12, 30), CMPUT350::Colors::white);  // body
    d->DrawCircle(CMPUT350::Point2D(x, y - 12), 6, CMPUT350::Colors::white);      // nose
    d->DrawCircle(CMPUT350::Point2D(x, y - 2), 4, CMPUT350::Colors::cyan);        // cockpit
    d->DrawRect(CMPUT350::Rect(x - 16, y + 12, 6, 6), CMPUT350::Colors::red);     // left engine
    d->DrawRect(CMPUT350::Rect(x + 10, y + 12, 6, 6), CMPUT350::Colors::red);     // right engine

}

/**
 * @brief Bullets do not hurt the player in 1a, so collisions are ignored.
 * @param obj The object collided with.
 */
void Player::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
}

/**
 * @brief Marks the player as dead.
 */
void Player::Kill()
{
     mAlive = false;
}

/**
 * @brief Whether the player should stay in the engine.
 * @return true unless Kill() was called.
 */
bool Player::IsAlive() const
{
    return mAlive;
}

/**
 * @brief The ship's bounding box, used for collisions.
 * @return The 40x40 box around the ship.
 */
const CMPUT350::Rect& Player::GetBounds(){
    return mBounds;
}
