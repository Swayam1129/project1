#include "Galaga.h"

#include <algorithm>
#include <unordered_map>

#include "Enemy.h"
#include "Player.h"
#include "Stars.h"

// How long the timed states last, in frames (the engine runs at 30 FPS)
static const int kStartupFrames = 90;
static const int kRespawnFrames = 90;
static const int kGameOverFrames = 150;
static const int kStartingLives = 3;

// Points for each "enemy destroyed" notification. Galaga registers for every
// name in this table, so a new enemy type only needs a new row here.
static const std::unordered_map<std::string, int> kEnemyPoints = {
    {"EnemyKilled", 50},
};

/**
 * @brief Creates the game manager.
 * @param width Window width in pixels.
 * @param height Window height in pixels.
 */
Galaga::Galaga(int width, int height)
    : mWidth(width), mHeight(height), mState(State::kWaitingForCoin), mStateStartFrame(0),
      mCredits(0), mScore(0), mHighScore(0), mLives(0), mLevel(1), mEnemiesAlive(0),
      mStartRequested(false), mPlayerDied(false)
{
}

/**
 * @brief Sets up the key -> notification mapping in the engine, registers for
 *        the notifications Galaga needs, and adds the star background.
 * @param context Game context (CurrObject is this Galaga object).
 */
void Galaga::Initialize(CMPUT350::GameContext* context)
{
    using Scan = sf::Keyboard::Scan;
    CMPUT350::EngineView *engine = context->mEngineView;
    engine->InstallKeyDownNotification(static_cast<int>(Scan::A), "MoveLeft");
    engine->InstallKeyDownNotification(static_cast<int>(Scan::Left), "MoveLeft");
    engine->InstallKeyUpNotification(static_cast<int>(Scan::A), "StopLeft");
    engine->InstallKeyUpNotification(static_cast<int>(Scan::Left), "StopLeft");
    engine->InstallKeyDownNotification(static_cast<int>(Scan::D), "MoveRight");
    engine->InstallKeyDownNotification(static_cast<int>(Scan::Right), "MoveRight");
    engine->InstallKeyUpNotification(static_cast<int>(Scan::D), "StopRight");
    engine->InstallKeyUpNotification(static_cast<int>(Scan::Right), "StopRight");
    engine->InstallKeyDownNotification(static_cast<int>(Scan::Space), "Fire");
    engine->InstallKeyDownNotification(static_cast<int>(Scan::Num5), "InsertCoin");   // MAME: 5 = coin
    engine->InstallKeyDownNotification(static_cast<int>(Scan::Num1), "Start");        // MAME: 1 = start

    CMPUT350::NotificationManager *nm = context->mNotificationManager;
    nm->Register(context->CurrObject, "InsertCoin");
    nm->Register(context->CurrObject, "Start");
    nm->Register(context->CurrObject, "PlayerDied");
    for (const auto &entry : kEnemyPoints)
    {
        nm->Register(context->CurrObject, entry.first);
    }

    engine->AddGameObject(std::make_shared<Stars>(250, CMPUT350::Rect(0, 0, mWidth, mHeight)));
    SetState(State::kWaitingForCoin, context);
}

/**
 * @brief Handles notifications. Only records what happened; the state
 *        machine acts on it in Update, which has the game context.
 * @param key The notification name.
 */
void Galaga::ReceiveNotification(const std::string &key)
{
    if (key == "InsertCoin")
    {
        mCredits++;
    }
    else if (key == "Start")
    {
        mStartRequested = true;
    }
    else if (key == "PlayerDied")
    {
        mPlayerDied = true;
    }
    else
    {
        auto it = kEnemyPoints.find(key);
        if (it != kEnemyPoints.end())
        {
            mScore += it->second;
            mEnemiesAlive--;
        }
    }
}

/**
 * @brief Runs one step of the state machine.
 * @param context Game context.
 */
void Galaga::Update(CMPUT350::GameContext* context)
{
    mHighScore = std::max(mHighScore, mScore);

    switch (mState)
    {
        case State::kWaitingForCoin:
            if (mStartRequested && mCredits > 0)
            {
                mCredits--;
                StartGame(context);
            }
            break;

        case State::kStartup:
            if (FramesInState(context) >= kStartupFrames)
            {
                SpawnWaves(context);
                SetState(State::kWaves, context);
            }
            break;

        case State::kWaves:
            // All enemies are placed at once for now; flying in comes with the Enemy classes
            SetState(State::kPlay, context);
            break;

        case State::kPlay:
            if (mPlayerDied)
            {
                mPlayerDied = false;
                mLives--;
                SetState(mLives > 0 ? State::kRespawn : State::kGameOver, context);
            }
            else if (mEnemiesAlive <= 0)
            {
                mLevel++;
                SetState(State::kStartup, context);   // next stage
            }
            break;

        case State::kRespawn:
            if (FramesInState(context) >= kRespawnFrames)
            {
                SpawnPlayer(context);
                SetState(State::kPlay, context);
            }
            break;

        case State::kGameOver:
            if (FramesInState(context) >= kGameOverFrames)
            {
                ClearGameObjects(context);
                SetState(State::kWaitingForCoin, context);
            }
            break;
    }
    mStartRequested = false;   // "Start" only counts on the title screen
}

/**
 * @brief Switches state and remembers the frame it happened on, so timed
 *        states know how long they have been running.
 * @param state The new state.
 * @param context Game context (current frame).
 */
void Galaga::SetState(State state, CMPUT350::GameContext* context)
{
    mState = state;
    mStateStartFrame = context->currentFrame;
}

/**
 * @brief How many frames the game has been in the current state.
 * @param context Game context (current frame).
 * @return Frames since the last SetState.
 */
int Galaga::FramesInState(CMPUT350::GameContext* context) const
{
    return context->currentFrame - mStateStartFrame;
}

/**
 * @brief Resets score, lives and level, brings in the player and shows stage 1.
 * @param context Game context.
 */
void Galaga::StartGame(CMPUT350::GameContext* context)
{
    mScore = 0;
    mLives = kStartingLives;
    mLevel = 1;
    mEnemiesAlive = 0;
    mPlayerDied = false;
    SpawnPlayer(context);
    SetState(State::kStartup, context);
}

/**
 * @brief Adds a new player ship at the bottom center, unless one is in play.
 * @param context Game context (used to add the ship to the engine).
 */
void Galaga::SpawnPlayer(CMPUT350::GameContext* context)
{
    if (!mPlayer.expired())
    {
        return;
    }
    auto player = std::make_shared<Player>(CMPUT350::Point2D(mWidth / 2.0f, mHeight - 100.0f));
    context->mEngineView->AddGameObject(player);
    mPlayer = player;
}

/**
 * @brief Adds the enemies for this stage: 5 rows of 8 in formation.
 * @param context Game context (used to add enemies to the engine).
 */
void Galaga::SpawnWaves(CMPUT350::GameContext* context)
{
    const int kRows = 5;
    const int kCols = 8;
    for (int row = 0; row < kRows; row++)
    {
        for (int col = 0; col < kCols; col++)
        {
            float x = mWidth / 2.0f + (col - (kCols - 1) / 2.0f) * 60;   // centered columns, 60 px apart
            float y = 100 + row * 45;
            context->mEngineView->AddGameObject(std::make_shared<Enemy>(CMPUT350::Point2D(x, y)));
            mEnemiesAlive++;
        }
    }
}

/**
 * @brief Removes the player, enemies and bullets from the engine after a game.
 *        Uses the engine's object iterator; Kill() does nothing on objects
 *        that should stay (the stars), and Galaga skips itself.
 * @param context Game context (CurrObject is this Galaga object).
 */
void Galaga::ClearGameObjects(CMPUT350::GameContext* context)
{
    std::shared_ptr<CMPUT350::GameObject> self = context->CurrObject.lock();
    for (const auto &obj : *context->mEngineView)
    {
        if (obj != self)
        {
            obj->Kill();
        }
    }
    mEnemiesAlive = 0;
}

/**
 * @brief Draws the score, lives and the text for the current state on the
 *        GUI context (plain screen coordinates, no object transform).
 * @param context Game context.
 */
void Galaga::RenderUI(CMPUT350::GameContext* context)
{
    CMPUT350::DrawContext *gui = context->GUIContext;
    float cx = mWidth / 2.0f;
    float cy = mHeight / 2.0f;

    // Always shown: scores across the top
    gui->DrawText("SCORE", 20, CMPUT350::Point2D(20, 10), CMPUT350::Colors::red);
    gui->DrawText(std::to_string(mScore), 20, CMPUT350::Point2D(20, 34), CMPUT350::Colors::white);
    gui->DrawCenteredText("HIGH SCORE", 20, CMPUT350::Point2D(cx, 20), CMPUT350::Colors::red);
    gui->DrawCenteredText(std::to_string(mHighScore), 20, CMPUT350::Point2D(cx, 44), CMPUT350::Colors::white);

    switch (mState)
    {
        case State::kWaitingForCoin:
            gui->DrawCenteredText("GALAGA", 64, CMPUT350::Point2D(cx, cy - 120), CMPUT350::Colors::cyan);
            gui->DrawCenteredText("PRESS 5 TO INSERT COIN", 24, CMPUT350::Point2D(cx, cy), CMPUT350::Colors::white);
            gui->DrawCenteredText("PRESS 1 TO START", 24, CMPUT350::Point2D(cx, cy + 40), CMPUT350::Colors::white);
            gui->DrawCenteredText("CREDITS " + std::to_string(mCredits), 20, CMPUT350::Point2D(cx, mHeight - 40),
                                  CMPUT350::Colors::yellow);
            return;   // no lives/stage on the title screen
        case State::kStartup:
            gui->DrawCenteredText("STAGE " + std::to_string(mLevel), 32, CMPUT350::Point2D(cx, cy),
                                  CMPUT350::Colors::cyan);
            break;
        case State::kRespawn:
            gui->DrawCenteredText("READY", 32, CMPUT350::Point2D(cx, cy), CMPUT350::Colors::cyan);
            gui->DrawCenteredText("SHIPS LEFT " + std::to_string(mLives), 24, CMPUT350::Point2D(cx, cy + 40),
                                  CMPUT350::Colors::white);
            break;
        case State::kGameOver:
            gui->DrawCenteredText("GAME OVER", 48, CMPUT350::Point2D(cx, cy), CMPUT350::Colors::red);
            break;
        default:
            break;
    }

    // During a game: ships left (bottom left) and stage (bottom right)
    gui->DrawText("SHIPS " + std::to_string(mLives), 20, CMPUT350::Point2D(20, mHeight - 34),
                  CMPUT350::Colors::white);
    gui->DrawText("STAGE " + std::to_string(mLevel), 20, CMPUT350::Point2D(mWidth - 130, mHeight - 34),
                  CMPUT350::Colors::white);
}
