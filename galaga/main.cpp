#include "GameEngine.h"
#include "Galaga.h"

int main()
{
    // Spec size is 768x1024; the height is reduced to 850 so the whole
    // window fits on a laptop screen (the spec allows a smaller size).
    const int kWidth = 768;
    const int kHeight = 850;

    CMPUT350::GameEngine engine(kWidth, kHeight, "Galaga");
    auto game = std::make_shared<Galaga>(kWidth, kHeight);
    engine.AddGameObject(game);
    engine.Run();

    return 0;
}
