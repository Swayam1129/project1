#include "Bullet.h"

#include "Enemy.h"
#include "Player.h"

static const int kExplodeFrames = 6;        // how long the hit circle is shown
static const float kExplodeRadius = 6.0f;
static const CMPUT350::RGBColor kPlayerBulletColor = CMPUT350::Colors::yellow;
static const CMPUT350::RGBColor kEnemyBulletColor(255, 140, 0);   // orange

/**
 * @brief Creates a bullet.
 * @param location Starting position in pixels.
 * @param heading Movement per frame in pixels (e.g. (0,-15) moves up 15 px/frame).
 * @param player true if the bullet was fired by the player, false for an enemy.
 * @param parent The object that fired it; the bullet dies when it is destroyed.
 */
Bullet::Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player,
               std::weak_ptr<CMPUT350::GameObject> parent)
    : mLoc(location), mPrevLoc(location), mHeading(heading), mPlayer(player), mAlive(true),
      mParent(parent), mExploding(false), mExplodeFrames(0), mHitPoint(0, 0),
      mBounds(CMPUT350::Point2D(0, 0), CMPUT350::Point2D(0, 0))
{
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Line(CMPUT350::Point2D(0, 0), CMPUT350::Point2D(0, 0))));
}

/**
 * @brief Whether the player fired this bullet.
 * @return true for a player bullet, false for an enemy bullet.
 */
bool Bullet::IsPlayerBullet()
{
    return mPlayer;
}

/**
 * @brief Moves the bullet one step and updates its bounds and line shape, or
 *        counts down the explosion if it has hit something.
 *
 * The line covers both the previous and current positions, so a fast bullet
 * cannot pass through an object between frames. The bullet dies when it
 * leaves the screen or when the object that fired it is gone.
 *
 * @param context Game context (used for the window size).
 */
void Bullet::Update(CMPUT350::GameContext* context)
{
    std::shared_ptr<CMPUT350::GameObject> parent = mParent.lock();
    if (parent == nullptr || !parent->IsAlive())
    {
        mAlive = false;   // shooter destroyed: its bullets go too
        return;
    }

    if (mExploding)
    {
        mExplodeFrames--;
        if (mExplodeFrames <= 0)
        {
            mAlive = false;
        }
        return;   // stays where it hit
    }

    mPrevLoc = mLoc;
    mLoc += mHeading;
    mBounds = CMPUT350::Rect(mPrevLoc - mLoc, CMPUT350::Point2D(0, 0));   // local: last position -> here
    // Directed from where we were to where we are, so its first hit is the first contact
    mShapes[0] = CMPUT350::Shape(CMPUT350::Line(mPrevLoc - mLoc, CMPUT350::Point2D(0, 0)));

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
void Bullet::LateUpdate(CMPUT350::GameContext* context)
{
}

/**
 * @brief Draws the bullet: a line along this frame's movement, or after a hit
 *        a single circle at the contact point. Player and enemy bullets use
 *        different colors.
 * @param context Game context (used for its DrawContext).
 */
void Bullet::RenderForeground(CMPUT350::GameContext* context)
{
    CMPUT350::RGBColor color = mPlayer ? kPlayerBulletColor : kEnemyBulletColor;
    if (mExploding)
    {
        context->ScreenContext->DrawCircle(mHitPoint, kExplodeRadius, color);
    }
    else
    {
        context->ScreenContext->DrawLine(mPrevLoc - mLoc, CMPUT350::Point2D(0, 0), 3, color);
    }
}

/**
 * @brief Whether an object is something this bullet should stop on:
 *        enemies for player bullets, the player for enemy bullets.
 * @param obj The object collided with.
 * @return true if the bullet hits it.
 */
bool Bullet::IsTarget(const std::shared_ptr<CMPUT350::CollisionObject>& obj) const
{
    bool hitEnemy = std::dynamic_pointer_cast<Enemy>(obj) != nullptr;
    bool hitPlayer = std::dynamic_pointer_cast<Player>(obj) != nullptr;
    return (mPlayer && hitEnemy) || (!mPlayer && hitPlayer);
}

/**
 * @brief Collision without a contact point (not expected for a line, but
 *        required by the interface): explode where the bullet is now.
 * @param obj The object collided with.
 */
void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    CollisionEnter(obj, CMPUT350::Point2D(0, 0));
}

/**
 * @brief Called by the engine with the first point where the bullet's line
 *        touched the object, in the bullet's local space. The bullet stops
 *        there and explodes. If it hits several objects in the same frame,
 *        the point with the largest y is kept.
 * @param obj The object collided with.
 * @param collisionPoint Contact point in local space.
 */
void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj,
                            const CMPUT350::Point2D &collisionPoint)
{
    if (!IsTarget(obj))
    {
        return;
    }
    if (!mExploding || collisionPoint.y > mHitPoint.y)
    {
        mHitPoint = collisionPoint;
    }
    mExploding = true;
    mExplodeFrames = kExplodeFrames;
}

/**
 * @brief Removes the bullet immediately (used when Galaga clears the game).
 */
void Bullet::Kill()
{
    mAlive = false;
}

/**
 * @brief Whether the bullet should stay in the engine.
 * @return true until it finishes exploding, leaves the screen, or is killed.
 */
bool Bullet::IsAlive() const
{
    return mAlive;
}

/**
 * @brief The bullet's bounding box, used for collisions.
 * @return A box around the previous and current positions (local space).
 */
const CMPUT350::Rect& Bullet::GetBounds()
{
    return mBounds;
}

/**
 * @brief The bullet's current position in screen space.
 * @return Current location.
 */
CMPUT350::Point2D Bullet::GetLocation() const
{
    return mLoc;
}

/**
 * @brief Bullets are not rotated; their heading is in the line they draw.
 * @return 0.
 */
float Bullet::GetRotation() const
{
    return 0;
}

/**
 * @brief The bullet's collision shape: a directed line from its previous
 *        position to its current one, in local space. Empty while exploding,
 *        so it can't hit anything else.
 * @return Shapes checked by the engine after the bounding boxes overlap.
 */
const std::vector<CMPUT350::Shape> &Bullet::GetShapes()
{
    return mExploding ? mNoShapes : mShapes;
}
