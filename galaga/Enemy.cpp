#include "Enemy.h"
#include "Bullet.h"

// Enemy size in pixels
static const int kEnemyWidth = 40;
static const int kEnemyHeight = 30;

/**
 * @brief Creates a stationary enemy.
 * @param loc The center of the enemy in pixels.
 */
Enemy::Enemy(CMPUT350::Point2D loc)
    : mLoc(loc), mBounds(CMPUT350::Point2D(-kEnemyWidth / 2.0f, -kEnemyHeight / 2.0f), kEnemyWidth, kEnemyHeight),
      mAlive(true)
{
}

/**
 * @brief Nothing to set up when the enemy is added to the engine.
 * @param context Game context.
 */
void Enemy::Initialize(CMPUT350::GameContext* context){}

/**
 * @brief Enemies do not move in 1a, so there is nothing to update.
 * @param context Game context.
 */
void Enemy::Update(CMPUT350::GameContext* context){}

/**
 * @brief No late update needed for enemies.
 * @param context Game context.
 */
void Enemy::LateUpdate(CMPUT350::GameContext* context){}

/**
 * @brief Enemies have no background layer.
 * @param context Game context.
 */
void Enemy::RenderBackground(CMPUT350::GameContext* context){}

/**
 * @brief Draws the enemy as a filled rectangle.
 * @param context Game context (used for its DrawContext).
 */
void Enemy::RenderForeground(CMPUT350::GameContext* context)
{
    context->ScreenContext->DrawRect(mBounds, CMPUT350::Colors::green);
}

/**
 * @brief Called by the engine when this enemy overlaps another collision object.
 *        The enemy dies only if it was hit by a player bullet; it is then
 *        removed by the engine at the start of the next frame.
 * @param obj The object collided with.
 */
void Enemy::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    std::shared_ptr<Bullet> bullet = std::dynamic_pointer_cast<Bullet>(obj);
    if (bullet != nullptr && bullet->IsPlayerBullet())
    {
        mAlive = false;   // engine removes us at the start of next frame
    }
}
/** @brief Marks the enemy as dead. */
void Enemy::Kill()
{
    mAlive = false;
}

/**
 * @brief Whether the enemy should stay in the engine.
 * @return true until the enemy has been hit.
 */

bool Enemy::IsAlive() const
{
    return mAlive;
}

/**
 * @brief The enemy's bounding box, used for drawing and collisions.
 * @return A reference to the box (a member, so it stays valid).
 */
const CMPUT350::Rect& Enemy::GetBounds()
{
     return mBounds;
}

/**
 * @brief The enemy's center in screen space; the engine draws it here.
 * @return Current location.
 */
CMPUT350::Point2D Enemy::GetLocation() const
{
    return mLoc;
}

/**
 * @brief The enemy's rotation in radians (0 until enemies fly paths).
 * @return Current rotation.
 */
float Enemy::GetRotation() const
{
    return 0;
}
