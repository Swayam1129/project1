#ifndef GALAGA_H
#define GALAGA_H

#include <memory>
#include <string>

#include "GameContext.h"
#include "GameObject.h"

class Player;

/**
 * @brief Runs the whole game: a finite state machine that inserts coins,
 *        starts games, spawns the player and enemies, keeps score and lives,
 *        and draws the GUI. Adding one Galaga object to the engine starts everything.
 */
class Galaga : public CMPUT350::GameObject
{
public:
    Galaga(int width, int height);

    // GameObject Functions
    void Initialize(CMPUT350::GameContext* context) override;
    void Update(CMPUT350::GameContext* context) override;
    void RenderUI(CMPUT350::GameContext* context) override;
    void ReceiveNotification(const std::string &key) override;

private:
    enum class State
    {
        kWaitingForCoin,   // title screen: 5 inserts a coin, 1 starts
        kStartup,          // shows the stage number, player appears
        kWaves,            // enemies fly in
        kPlay,             // normal play until all enemies are destroyed
        kRespawn,          // player died; wait, then bring in a new ship
        kGameOver          // shows GAME OVER, then back to the title screen
    };

    void SetState(State state, CMPUT350::GameContext* context);
    int FramesInState(CMPUT350::GameContext* context) const;
    void StartGame(CMPUT350::GameContext* context);
    void SpawnPlayer(CMPUT350::GameContext* context);
    void SpawnWaves(CMPUT350::GameContext* context);
    void ClearGameObjects(CMPUT350::GameContext* context);

    int mWidth, mHeight;           // window size in pixels
    State mState;                  // current state of the game
    int mStateStartFrame;          // frame when the current state began
    int mCredits;                  // coins inserted and not yet used
    int mScore, mHighScore;
    int mLives;                    // ships left, including the one in play
    int mLevel;                    // stage number, starting at 1
    int mEnemiesAlive;             // enemies spawned and not yet destroyed
    bool mStartRequested;          // "Start" received since the last Update
    bool mPlayerDied;              // "PlayerDied" received since the last Update
    std::weak_ptr<Player> mPlayer; // the ship in play (owned by the engine)
};

#endif // GALAGA_H
