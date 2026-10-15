#include <algorithm>
#include <cassert>
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "GameContext.h"

static const float kPlayerSize = 40.0f;     // ship is about 40x40
static const float kMoveSpeed = 8.0f;       // pixels per frame while a move key is held
static const float kBulletSpeed = 15.0f;    // pixels per frame

/**
 * @brief Creates the player ship. Its bounds are fixed in local space
 *        (centered on (0,0)); the engine moves them using GetLocation().
 * @param loc The center of the ship in screen pixels.
 */
Player::Player(CMPUT350::Point2D loc)
    : mLoc(loc), mBounds(-kPlayerSize / 2, -kPlayerSize / 2, kPlayerSize, kPlayerSize), mAlive(true)
{
    // Collision shapes match the main parts of the drawing (see RenderForeground)
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Rect(-6, -12, 12, 30)));              // body
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Rect(-20, 2, 40, 10)));               // wings
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Circle(CMPUT350::Point2D(0, -12), 6))); // nose
}

/**
 * @brief Registers the player for the keyboard notifications the game
 *        installs in the engine (move left/right, stop, fire).
 * @param context Game context (CurrObject is this player).
 */
void Player::Initialize(CMPUT350::GameContext* context)
{
    CMPUT350::NotificationManager *nm = context->mNotificationManager;
    nm->Register(context->CurrObject, "MoveLeft");
    nm->Register(context->CurrObject, "StopLeft");
    nm->Register(context->CurrObject, "MoveRight");
    nm->Register(context->CurrObject, "StopRight");
    nm->Register(context->CurrObject, "Fire");
}

/**
 * @brief Moves the ship while a move key is held (kept inside the screen)
 *        and fires if "Fire" was received since the last frame.
 *
 * At most two player bullets can exist at once. Bullets are tracked with
 * weak_ptrs, which don't keep them alive: once the engine removes a bullet,
 * its weak_ptr expires and that slot can fire again.
 *
 * @param context Game context (screen width, adding bullets).
 */
void Player::Update(CMPUT350::GameContext* context)
{
    float half = kPlayerSize / 2;
    float maxX = context->ScreenContext->GetWindowWidth() - half;

    if (mMoveLeft)
    {
        mLoc.x -= kMoveSpeed;
    }
    if (mMoveRight)
    {
        mLoc.x += kMoveSpeed;
    }
    mLoc.x = std::clamp(mLoc.x, half, maxX);   // don't leave the screen

    if (mFireRequested)
    {
        mFireRequested = false;
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
    }
}

/**
 * @brief Tells Galaga (through the NotificationManager) that the ship was
 *        destroyed, so it can take a life and respawn or end the game.
 * @param context Game context.
 */
void Player::LateUpdate(CMPUT350::GameContext* context)
{
    if (mDied)
    {
        mDied = false;   // report only once
        context->mNotificationManager->Notify("PlayerDied");
    }
}

/**
 * @brief Handles keyboard notifications. Only records what was asked;
 *        the movement and firing happen in Update, which has the context.
 * @param key The notification name.
 */
void Player::ReceiveNotification(const std::string &key)
{
    if (key == "MoveLeft")
    {
        mMoveLeft = true;
    }
    else if (key == "StopLeft")
    {
        mMoveLeft = false;
    }
    else if (key == "MoveRight")
    {
        mMoveRight = true;
    }
    else if (key == "StopRight")
    {
        mMoveRight = false;
    }
    else if (key == "Fire")
    {
        mFireRequested = true;
    }
}

/**
 * @brief The player has no background layer.
 * @param context Game context.
 */
void Player::RenderBackground(CMPUT350::GameContext* context){}

/**
 * @brief Draws the ship around its local (0,0); the engine moves/rotates it.
 * @param context Game context (used for its DrawContext).
 */
void Player::RenderForeground(CMPUT350::GameContext* context)
{
    CMPUT350::DrawContext *d = context->ScreenContext;
    float x = 0, y = 0;   // local space: (0,0) is the ship's center

    d->DrawRect(CMPUT350::Rect(x - 20, y + 2, 40, 10), CMPUT350::Colors::gray);   // wings
    d->DrawRect(CMPUT350::Rect(x - 6, y - 12, 12, 30), CMPUT350::Colors::white);  // body
    d->DrawCircle(CMPUT350::Point2D(x, y - 12), 6, CMPUT350::Colors::white);      // nose
    d->DrawCircle(CMPUT350::Point2D(x, y - 2), 4, CMPUT350::Colors::cyan);        // cockpit
    d->DrawRect(CMPUT350::Rect(x - 16, y + 12, 6, 6), CMPUT350::Colors::red);     // left engine
    d->DrawRect(CMPUT350::Rect(x + 10, y + 12, 6, 6), CMPUT350::Colors::red);     // right engine

}

/**
 * @brief The ship's center in screen space; the engine draws it here.
 * @return Current location.
 */
CMPUT350::Point2D Player::GetLocation() const
{
    return mLoc;
}

/**
 * @brief The ship's rotation in radians (0 during normal play).
 * @return Current rotation.
 */
float Player::GetRotation() const
{
    return mRotation;
}

/**
 * @brief Called by the engine on a collision. An enemy bullet or an enemy
 *        destroys the ship; the player's own bullets are ignored.
 * @param obj The object collided with.
 */
void Player::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    std::shared_ptr<Bullet> bullet = std::dynamic_pointer_cast<Bullet>(obj);
    bool enemyBullet = bullet != nullptr && !bullet->IsPlayerBullet();
    bool enemy = std::dynamic_pointer_cast<Enemy>(obj) != nullptr;
    if (mAlive && (enemyBullet || enemy))
    {
        mAlive = false;   // engine removes us at the start of next frame
        mDied = true;     // Galaga is told in LateUpdate (we have no context here)
    }
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
 * @return The 40x40 box around (0,0) in local space.
 */
const CMPUT350::Rect& Player::GetBounds(){
    return mBounds;
}

/**
 * @brief The ship's collision shapes (body, wings, nose) in local space.
 * @return Shapes checked by the engine after the bounding boxes overlap.
 */
const std::vector<CMPUT350::Shape> &Player::GetShapes()
{
    return mShapes;
}
