#include "Bullet.h"
#include "Enemy.h"
/**
 * @brief Creates a bullet.
 * @param location Starting position in pixels.
 * @param heading Movement per frame in pixels (e.g. (0,-15) moves up 15 px/frame).
 * @param player true if the bullet was fired by the player, false for an enemy.
 */
Bullet::Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player)
                : mLoc(location), mPrevLoc(location), mHeading(heading),
                mPlayer(player), mAlive(true), mBounds(location, location)
{
}

/**
 * @brief Whether the player fired this bullet.
 * @return true for a player bullet, false for an enemy bullet.
 */
bool Bullet::IsPlayerBullet(){
    return mPlayer;
}

/**
 * @brief Nothing to set up when the bullet is added to the engine.
 * @param context Game context.
 */
void Bullet::Initialize(CMPUT350::GameContext* context){}

/**
 * @brief Moves the bullet one step and updates its bounding box.
 *
 * The box covers both the previous and current positions, so a fast bullet
 * cannot pass through an object between frames. The bullet dies when it
 * leaves the screen so it is removed from the engine.
 * 
 * @param context Game context (used for the window size).
 */
void Bullet::Update(CMPUT350::GameContext* context)
{
     mPrevLoc = mLoc;
    mLoc += mHeading;
    mBounds = CMPUT350::Rect(mPrevLoc, mLoc);

    int w = context->ScreenContext->GetWindowWidth();
    int h = context->ScreenContext->GetWindowHeight();
    if (mLoc.x < 0 || mLoc.x > w || mLoc.y < 0 || mLoc.y > h)
    {
        mAlive = false;
    }
}

/**
 * @brief No late update needed for bullets.
 * @param context Game context.
 */
void Bullet::LateUpdate(CMPUT350::GameContext* context){}

/**
 * @brief Bullets have no background layer.
 * @param context Game context.
 */
void Bullet::RenderBackground(CMPUT350::GameContext* context){}

/**
 * @brief Draws the bullet as a line along the path it travelled this frame.
 * @param context Game context (used for its DrawContext).
 */
void Bullet::RenderForeground(CMPUT350::GameContext* context)
{
    context->ScreenContext->DrawLine(mPrevLoc, mLoc, 3, CMPUT350::Colors::yellow);
}

/**
 * @brief Called by the engine when the bullet overlaps another collision object.
 *        A player bullet dies when it hits an Enemy. It ignores the player
 *        that fired it, so it does not explode when it spawns.
 * @param obj The object collided with.
 */
void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    if (mPlayer && std::dynamic_pointer_cast<Enemy>(obj) != nullptr)
    {
        mAlive = false;
    }
}

/**
 * @brief Marks the bullet as dead.
 */
void Bullet::Kill()
{
    mAlive = false;
}

/**
 * @brief Whether the bullet should stay in the engine.
 * @return true until the bullet hits an enemy or leaves the screen.
 */
bool Bullet::IsAlive() const
{
    return mAlive;
}

/**
 * @brief The bullet's bounding box, used for collisions.
 * @return A box around the previous and current positions.
 */
const CMPUT350::Rect& Bullet::GetBounds()
{
    return mBounds;
}
