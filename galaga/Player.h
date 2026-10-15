#ifndef PLAYER_H
#define PLAYER_H

#include "CollisionObject.h"
#include <memory>
class Bullet;
class Player : public CMPUT350::CollisionObject
{
public:
    Player(CMPUT350::Point2D loc);

    // GameObject Functions
    void Initialize(CMPUT350::GameContext* context) override;
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    void ReceiveNotification(const std::string &key) override;
    bool IsAlive() const override;
    void Kill() override;

    // Graphics Object Functions
    void RenderBackground(CMPUT350::GameContext* context) override;
    void RenderForeground(CMPUT350::GameContext* context) override;

    CMPUT350::Point2D GetLocation() const override;
    float GetRotation() const override;

    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
    const std::vector<CMPUT350::Shape> &GetShapes() override;
    
private:
    CMPUT350::Point2D mLoc;               // center of the ship
    CMPUT350::Rect mBounds;               // 40x40 box around (0,0), local space
    std::vector<CMPUT350::Shape> mShapes; // body, wings, nose in local space
    bool mAlive;
    float mRotation = 0;                  // always 0 in play; change to test rotation
    std::weak_ptr<Bullet> mBullets[2];    // watches (doesn't own) up to 2 bullets
    bool mMoveLeft = false;               // true while a MoveLeft key is held
    bool mMoveRight = false;              // true while a MoveRight key is held
    bool mFireRequested = false;          // set by "Fire", handled in Update
};

#endif
