#ifndef ENEMY_H
#define ENEMY_H

#include <string>
#include <vector>

#include "Bezier.h"
#include "CollisionObject.h"
#include "GameContext.h"

/**
 * @brief Base class for all enemies. Handles the shared behaviour: flying
 *        along a Bezier path at constant speed, joining and hovering in
 *        formation, taking damage, and reporting its death to Galaga.
 *        Each enemy type decides its own attack path, firing and look.
 */
class Enemy : public CMPUT350::CollisionObject
{
public:
    Enemy(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D> &path);

    // Asks the enemy to leave formation and attack the target.
    // Returns false if it can't right now (not in formation).
    virtual bool Attack(CMPUT350::GameContext *context, const CMPUT350::Point2D &target) = 0;
    bool IsInFormation() const;

    // GameObject Functions
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    bool IsAlive() const override;
    void Kill() override;

    // Graphics Object Functions
    CMPUT350::Point2D GetLocation() const override;
    float GetRotation() const override;

    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
    const std::vector<CMPUT350::Shape> &GetShapes() override;

protected:
    enum class State
    {
        kEntering,      // flying the entry path onto the screen
        kJoining,       // moving straight to its slot in the formation
        kInFormation,   // hovering in its slot
        kAttacking      // flying its attack path
    };

    void StartPath(const std::vector<CMPUT350::Point2D> &path, State state);
    void FireAt(CMPUT350::GameContext *context, const CMPUT350::Point2D &target);
    CMPUT350::Point2D FormationSlot(CMPUT350::GameContext *context) const;

    std::string mName;       // set by each type; used for "<name>Killed" notifications
    int mHealth;             // hits left; set by each type
    State mState;
    CMPUT350::Point2D mLoc;          // center in screen space
    CMPUT350::Point2D mFormationLoc; // slot in the formation (before hovering)
    CMPUT350::Point2D mTarget;       // where the current attack is aimed
    float mRotation;                 // radians, 0 = facing up
    CMPUT350::Bezier mPath;          // path currently being followed
    float mT;                        // position along mPath (0 .. mPath.GetMaxT())
    int mShotsFired;                 // bullets fired during the current attack

private:
    bool FollowPath();
    bool MoveToward(const CMPUT350::Point2D &dest);
    void FaceDirection(const CMPUT350::Point2D &dir);

    CMPUT350::Rect mBounds;               // box around (0,0), local space
    std::vector<CMPUT350::Shape> mShapes; // one rect = mBounds
    bool mAlive;                          // false once destroyed or killed
    bool mHit;                            // destroyed by the player (reported in LateUpdate)
    bool mHitOutOfFormation;              // was it attacking/flying when destroyed (more points)
};

/** @brief Small enemy with 1 health; dives straight at the player and fires once. */
class Bee : public Enemy
{
public:
    Bee(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D> &path);
    void LateUpdate(CMPUT350::GameContext *context) override;
    bool Attack(CMPUT350::GameContext *context, const CMPUT350::Point2D &target) override;
    void RenderForeground(CMPUT350::GameContext *context) override;
};

/** @brief Enemy with 1 health; swerves in an S-curve and fires twice. */
class Butterfly : public Enemy
{
public:
    Butterfly(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D> &path);
    void LateUpdate(CMPUT350::GameContext *context) override;
    bool Attack(CMPUT350::GameContext *context, const CMPUT350::Point2D &target) override;
    void RenderForeground(CMPUT350::GameContext *context) override;
};

/** @brief Large enemy with 2 health (changes color when hit); loops around
 *         the player and fires a 3-bullet spread. */
class Boss : public Enemy
{
public:
    Boss(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D> &path);
    void LateUpdate(CMPUT350::GameContext *context) override;
    bool Attack(CMPUT350::GameContext *context, const CMPUT350::Point2D &target) override;
    void RenderForeground(CMPUT350::GameContext *context) override;
};

#endif
