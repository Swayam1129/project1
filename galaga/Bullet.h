#ifndef BULLET_H
#define BULLET_H

#include <memory>
#include <vector>

#include "CollisionObject.h"
#include "GameContext.h"

/**
 * @brief A bullet fired by the player (upward) or an enemy (toward the player).
 *        Its collision shape is a directed line covering this frame's movement.
 *        On a hit it stops at the exact contact point and shows a small
 *        explosion circle there for a few frames. It also dies when the
 *        object that fired it is destroyed.
 */
class Bullet : public CMPUT350::CollisionObject
{
public:
    Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player,
           std::weak_ptr<CMPUT350::GameObject> parent);
    bool IsPlayerBullet();

    // GameObject Functions
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    bool IsAlive() const override;
    void Kill() override;

    // Graphics Object Functions
    void RenderForeground(CMPUT350::GameContext* context) override;
    CMPUT350::Point2D GetLocation() const override;
    float GetRotation() const override;

    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj,
                        const CMPUT350::Point2D &collisionPoint) override;
    const CMPUT350::Rect& GetBounds() override;
    const std::vector<CMPUT350::Shape> &GetShapes() override;

private:
    bool IsTarget(const std::shared_ptr<CMPUT350::CollisionObject>& obj) const;

    CMPUT350::Point2D mLoc;      // current position
    CMPUT350::Point2D mPrevLoc;  // position last frame
    CMPUT350::Point2D mHeading;  // movement per frame (direction * speed)
    bool mPlayer;                // true if fired by the player
    bool mAlive;                 // false once finished exploding, off screen, or killed
    std::weak_ptr<CMPUT350::GameObject> mParent;   // who fired it (not owned)
    bool mExploding;             // hit something; showing the explosion
    int mExplodeFrames;          // explosion frames left
    CMPUT350::Point2D mHitPoint; // contact point in local space
    CMPUT350::Rect mBounds;      // box from last position to (0,0), local space
    std::vector<CMPUT350::Shape> mShapes;      // one line: last position -> (0,0)
    std::vector<CMPUT350::Shape> mNoShapes;    // empty: an exploding bullet hits nothing
};

#endif // BULLET_H
