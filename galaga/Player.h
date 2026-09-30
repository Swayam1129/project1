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
    bool HandleKeyEvent(CMPUT350::GameContext* context, char key) override;
    bool IsAlive() const override;
    void Kill() override;

    // Graphics Object Functions
    void RenderBackground(CMPUT350::GameContext* context) override;
    void RenderForeground(CMPUT350::GameContext* context) override;


    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
    
private:
    CMPUT350::Point2D mLoc;               // center of the ship
    CMPUT350::Rect mBounds;               // 40x40 box around mLoc
    bool mAlive;
    std::weak_ptr<Bullet> mBullets[2];    // watches (doesn't own) up to 2 bullets

    void UpdateBounds();                  // rebuild mBounds after moving
};

#endif
