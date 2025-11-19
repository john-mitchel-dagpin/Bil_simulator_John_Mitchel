
#include "Game.hpp"
#include <threepp/threepp.hpp>

using namespace threepp;

int main() {

    Canvas canvas("Bilsimulator");
    GLRenderer renderer(canvas.size());
    renderer.setClearColor(Color::aliceblue);

    Game game(canvas, renderer);
    game.run();

    return 0;
}
