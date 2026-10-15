#include "Enemy.h"

#include <cmath>

#include "Bullet.h"
#include "Player.h"

// Enemy size in pixels
static const int kEnemyWidth = 40;
static const int kEnemyHeight = 30;

static const float kPi = 3.14159265f;
static const float kFlySpeed = 7.0f;        // pixels per frame along any path
static const float kHoverAmplitude = 15.0f; // formation sways this far left/right
static const float kHoverRate = 0.05f;      // radians per frame for the sway
static const float kBulletSpeed = 8.0f;     // enemy bullets, pixels per frame

// ---------------------------------------------------------------------------
// Enemy (shared behaviour)
// ---------------------------------------------------------------------------

/**
 * @brief Creates an enemy that will fly the given entry path and then take
 *        its place in the formation.
 * @param loc The enemy's slot in the formation (screen space).
 * @param path Bezier control points (k*3+1) for flying onto the screen.
 */
Enemy::Enemy(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D> &path)
    : mName("Enemy"), mHealth(1), mState(State::kEntering), mLoc(path.empty() ? loc : path[0]),
      mFormationLoc(loc), mRotation(0), mPath(path), mT(0), mShotsFired(0),
      mBounds(CMPUT350::Point2D(-kEnemyWidth / 2.0f, -kEnemyHeight / 2.0f), kEnemyWidth, kEnemyHeight),
      mAlive(true), mHit(false), mHitOutOfFormation(false)
{
    mShapes.push_back(CMPUT350::Shape(mBounds));
}

/**
 * @brief Whether the enemy is resting in its formation slot (not entering,
 *        joining or attacking).
 * @return true if in formation.
 */
bool Enemy::IsInFormation() const
{
    return mState == State::kInFormation;
}

/**
 * @brief The enemy's formation slot this frame. All enemies sway together
 *        because the sway uses the shared frame counter.
 * @param context Game context (current frame).
 * @return Slot position in screen space.
 */
CMPUT350::Point2D Enemy::FormationSlot(CMPUT350::GameContext *context) const
{
    float sway = kHoverAmplitude * std::sin(context->currentFrame * kHoverRate);
    return mFormationLoc + CMPUT350::Point2D(sway, 0);
}

/**
 * @brief Starts following a new Bezier path from its beginning.
 * @param path Control points (k*3+1).
 * @param state State to be in while following it.
 */
void Enemy::StartPath(const std::vector<CMPUT350::Point2D> &path, State state)
{
    mPath = CMPUT350::Bezier(path);
    mT = 0;
    mState = state;
}

/**
 * @brief Rotates the enemy so its nose (drawn pointing up) faces a direction.
 * @param dir Direction of travel.
 */
void Enemy::FaceDirection(const CMPUT350::Point2D &dir)
{
    if (dir.x != 0 || dir.y != 0)
    {
        mRotation = std::atan2(dir.y, dir.x) + kPi / 2;   // +90 degrees: "up" is the nose
    }
}

/**
 * @brief Moves along mPath at a constant speed of kFlySpeed pixels per frame.
 *        The slope (derivative) tells how many pixels one unit of t covers
 *        here, so t advances by speed / |slope|. This keeps the speed even
 *        when control points are bunched up or spread out.
 * @return true when the end of the path is reached.
 */
bool Enemy::FollowPath()
{
    CMPUT350::Point2D slope = mPath.GetSlope(mT);
    float pixelsPerT = static_cast<float>(slope.Distance(CMPUT350::Point2D(0, 0)));
    mT += (pixelsPerT > 0.001f) ? kFlySpeed / pixelsPerT : 0.01f;

    bool done = mT >= mPath.GetMaxT();
    if (done)
    {
        mT = mPath.GetMaxT();
    }
    mLoc = mPath.GetPoint(mT);
    FaceDirection(slope);
    return done;
}

/**
 * @brief Moves in a straight line toward a point at kFlySpeed.
 * @param dest Destination in screen space.
 * @return true once the destination is reached.
 */
bool Enemy::MoveToward(const CMPUT350::Point2D &dest)
{
    CMPUT350::Point2D d = dest - mLoc;
    float dist = static_cast<float>(d.Distance(CMPUT350::Point2D(0, 0)));
    if (dist <= kFlySpeed)
    {
        mLoc = dest;
        return true;
    }
    FaceDirection(d);
    mLoc += d * (kFlySpeed / dist);
    return false;
}

/**
 * @brief Moves the enemy for its current state: entering, joining the
 *        formation, hovering, or attacking. After an attack that leaves the
 *        bottom of the screen it reappears at the top and flies back to its slot.
 * @param context Game context.
 */
void Enemy::Update(CMPUT350::GameContext* context)
{
    switch (mState)
    {
        case State::kEntering:
            if (FollowPath())
            {
                mState = State::kJoining;
            }
            break;

        case State::kJoining:
            if (MoveToward(FormationSlot(context)))
            {
                mState = State::kInFormation;
            }
            break;

        case State::kInFormation:
            mLoc = FormationSlot(context);
            mRotation = 0;
            break;

        case State::kAttacking:
            if (FollowPath())
            {
                if (mLoc.y > context->ScreenContext->GetWindowHeight())
                {
                    mLoc.y = -kEnemyHeight;   // wrap to the top, like the arcade game
                }
                mState = State::kJoining;
            }
            break;
    }
}

/**
 * @brief Tells Galaga (through the NotificationManager) that this enemy was
 *        destroyed, so it can add points and count enemies. The name is
 *        "<type>Killed", plus "Attacking" if it was out of formation.
 * @param context Game context.
 */
void Enemy::LateUpdate(CMPUT350::GameContext* context)
{
    if (mHit)
    {
        mHit = false;   // report only once
        context->mNotificationManager->Notify(mName + "Killed" + (mHitOutOfFormation ? "Attacking" : ""));
    }
}

/**
 * @brief Fires a bullet from the enemy's nose toward a target.
 * @param context Game context (used to add the bullet).
 * @param target Point to aim at, in screen space.
 */
void Enemy::FireAt(CMPUT350::GameContext *context, const CMPUT350::Point2D &target)
{
    CMPUT350::Point2D dir = target - mLoc;
    dir.Normalize();
    if (dir.y < 0.3f)
    {
        dir = CMPUT350::Point2D(dir.x, 0.3f);   // always shoot mostly downward
        dir.Normalize();
    }
    auto bullet = std::make_shared<Bullet>(mLoc + dir * (kEnemyHeight / 2.0f + 2), dir * kBulletSpeed, false);
    context->mEngineView->AddGameObject(bullet);
    mShotsFired++;
}

/**
 * @brief Called by the engine on a collision. A player bullet removes one
 *        health; touching the player destroys the enemy. Either way, when
 *        it is destroyed Galaga is told in LateUpdate (no context here).
 * @param obj The object collided with.
 */
void Enemy::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    if (!mAlive)
    {
        return;
    }
    std::shared_ptr<Bullet> bullet = std::dynamic_pointer_cast<Bullet>(obj);
    if (bullet != nullptr && bullet->IsPlayerBullet())
    {
        mHealth--;
    }
    else if (std::dynamic_pointer_cast<Player>(obj) != nullptr)
    {
        mHealth = 0;
    }
    if (mHealth <= 0)
    {
        mAlive = false;   // engine removes us at the start of next frame
        mHit = true;
        mHitOutOfFormation = !IsInFormation();
    }
}

/** @brief Marks the enemy as dead (no points; used when Galaga clears the game). */
void Enemy::Kill()
{
    mAlive = false;
}

/**
 * @brief Whether the enemy should stay in the engine.
 * @return true until the enemy is destroyed or killed.
 */
bool Enemy::IsAlive() const
{
    return mAlive;
}

/**
 * @brief The enemy's bounding box in local space.
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
 * @brief The enemy's rotation in radians: it faces its direction of travel,
 *        and 0 (facing up) in formation.
 * @return Current rotation.
 */
float Enemy::GetRotation() const
{
    return mRotation;
}

/**
 * @brief The enemy's collision shapes in local space (one rect).
 * @return Shapes checked by the engine after the bounding boxes overlap.
 */
const std::vector<CMPUT350::Shape> &Enemy::GetShapes()
{
    return mShapes;
}

// ---------------------------------------------------------------------------
// Bee: 1 health, straight dive, fires once
// ---------------------------------------------------------------------------

/**
 * @brief Creates a Bee.
 * @param loc Formation slot.
 * @param path Entry path.
 */
Bee::Bee(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D> &path)
    : Enemy(loc, path)
{
    mName = "Bee";
    mHealth = 1;
}

/**
 * @brief Dives down past the player: a small hop up, then a curve onto the
 *        player's column and off the bottom of the screen.
 * @param context Game context (screen height).
 * @param target The player's location.
 * @return true if the attack started (the Bee was in formation).
 */
bool Bee::Attack(CMPUT350::GameContext *context, const CMPUT350::Point2D &target)
{
    if (!IsInFormation())
    {
        return false;
    }
    float bottom = context->ScreenContext->GetWindowHeight() + 60.0f;
    mTarget = target;
    mShotsFired = 0;
    StartPath({mLoc, mLoc + CMPUT350::Point2D(0, -60), CMPUT350::Point2D(target.x, mLoc.y + 150),
               CMPUT350::Point2D(target.x, bottom)},
              State::kAttacking);
    return true;
}

/**
 * @brief Fires one bullet a third of the way into its dive.
 * @param context Game context.
 */
void Bee::LateUpdate(CMPUT350::GameContext *context)
{
    Enemy::LateUpdate(context);
    if (mState == State::kAttacking && mShotsFired == 0 && mT > 0.35f)
    {
        FireAt(context, mTarget);
    }
}

/**
 * @brief Draws a yellow body with blue wings, nose up.
 * @param context Game context.
 */
void Bee::RenderForeground(CMPUT350::GameContext *context)
{
    CMPUT350::DrawContext *d = context->ScreenContext;
    d->DrawRect(CMPUT350::Rect(-18, -4, 36, 8), CMPUT350::Colors::blue);                 // wings
    d->DrawCircle(CMPUT350::Point2D(0, 0), 9, CMPUT350::Colors::yellow);                // body
    d->DrawCircle(CMPUT350::Point2D(0, -10), 5, CMPUT350::Colors::red);                 // head
}

// ---------------------------------------------------------------------------
// Butterfly: 1 health, S-curve, fires twice
// ---------------------------------------------------------------------------

/**
 * @brief Creates a Butterfly.
 * @param loc Formation slot.
 * @param path Entry path.
 */
Butterfly::Butterfly(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D> &path)
    : Enemy(loc, path)
{
    mName = "Butterfly";
    mHealth = 1;
}

/**
 * @brief Swerves out to one side, back across, and down past the player
 *        (two Bezier segments making an S).
 * @param context Game context (screen size).
 * @param target The player's location.
 * @return true if the attack started (the Butterfly was in formation).
 */
bool Butterfly::Attack(CMPUT350::GameContext *context, const CMPUT350::Point2D &target)
{
    if (!IsInFormation())
    {
        return false;
    }
    float bottom = context->ScreenContext->GetWindowHeight() + 60.0f;
    float side = (mLoc.x < context->ScreenContext->GetWindowWidth() / 2.0f) ? -1.0f : 1.0f;   // swerve outward first
    mTarget = target;
    mShotsFired = 0;
    StartPath({mLoc, mLoc + CMPUT350::Point2D(side * 80, -50), mLoc + CMPUT350::Point2D(side * 150, 120),
               CMPUT350::Point2D((mLoc.x + target.x) / 2, (mLoc.y + target.y) / 2),
               CMPUT350::Point2D(target.x - side * 200, target.y - 200), CMPUT350::Point2D(target.x + side * 100, target.y),
               CMPUT350::Point2D(target.x, bottom)},
              State::kAttacking);
    return true;
}

/**
 * @brief Fires at the start of each half of the S.
 * @param context Game context.
 */
void Butterfly::LateUpdate(CMPUT350::GameContext *context)
{
    Enemy::LateUpdate(context);
    if (mState == State::kAttacking && ((mShotsFired == 0 && mT > 0.6f) || (mShotsFired == 1 && mT > 1.2f)))
    {
        FireAt(context, mTarget);
    }
}

/**
 * @brief Draws a red body with white wings, nose up.
 * @param context Game context.
 */
void Butterfly::RenderForeground(CMPUT350::GameContext *context)
{
    CMPUT350::DrawContext *d = context->ScreenContext;
    d->DrawRect(CMPUT350::Rect(-20, -10, 14, 20), CMPUT350::Colors::white);    // left wing
    d->DrawRect(CMPUT350::Rect(6, -10, 14, 20), CMPUT350::Colors::white);      // right wing
    d->DrawRect(CMPUT350::Rect(-5, -14, 10, 28), CMPUT350::Colors::red);       // body
    d->DrawCircle(CMPUT350::Point2D(0, -12), 5, CMPUT350::Colors::yellow);     // head
}

// ---------------------------------------------------------------------------
// Boss: 2 health (changes color), loops around the player, 3-bullet spread
// ---------------------------------------------------------------------------

/**
 * @brief Creates a Boss.
 * @param loc Formation slot.
 * @param path Entry path.
 */
Boss::Boss(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D> &path)
    : Enemy(loc, path)
{
    mName = "Boss";
    mHealth = 2;
}

/**
 * @brief Swoops down beside the player, loops under and around it, and flies
 *        back up off the top of the screen (two Bezier segments).
 * @param context Game context (screen size).
 * @param target The player's location.
 * @return true if the attack started (the Boss was in formation).
 */
bool Boss::Attack(CMPUT350::GameContext *context, const CMPUT350::Point2D &target)
{
    if (!IsInFormation())
    {
        return false;
    }
    mTarget = target;
    mShotsFired = 0;
    StartPath({mLoc, mLoc + CMPUT350::Point2D(120, 100), CMPUT350::Point2D(target.x + 220, target.y - 60),
               CMPUT350::Point2D(target.x, target.y - 120), CMPUT350::Point2D(target.x - 220, target.y - 180),
               CMPUT350::Point2D(mLoc.x - 150, 150), CMPUT350::Point2D(mLoc.x, -40)},
              State::kAttacking);
    return true;
}

/**
 * @brief Fires a 3-bullet spread when it is closest to the player
 *        (the middle of its loop).
 * @param context Game context.
 */
void Boss::LateUpdate(CMPUT350::GameContext *context)
{
    Enemy::LateUpdate(context);
    if (mState == State::kAttacking && mShotsFired == 0 && mT > 0.9f)
    {
        FireAt(context, mTarget + CMPUT350::Point2D(-80, 0));
        FireAt(context, mTarget);
        FireAt(context, mTarget + CMPUT350::Point2D(80, 0));
    }
}

/**
 * @brief Draws a larger ship: green at full health, magenta after one hit.
 * @param context Game context.
 */
void Boss::RenderForeground(CMPUT350::GameContext *context)
{
    CMPUT350::DrawContext *d = context->ScreenContext;
    CMPUT350::RGBColor body = (mHealth >= 2) ? CMPUT350::Colors::green : CMPUT350::Colors::magenta;
    d->DrawRect(CMPUT350::Rect(-20, -6, 40, 12), CMPUT350::Colors::cyan);      // wings
    d->DrawRect(CMPUT350::Rect(-9, -15, 18, 30), body);                        // body
    d->DrawCircle(CMPUT350::Point2D(-6, -8), 3, CMPUT350::Colors::white);      // eyes
    d->DrawCircle(CMPUT350::Point2D(6, -8), 3, CMPUT350::Colors::white);
}
