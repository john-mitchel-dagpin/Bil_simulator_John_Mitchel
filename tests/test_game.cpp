#include <catch2/catch_test_macros.hpp>
#include "Game.hpp"
#include "InputState.hpp"

TEST_CASE("Game update delegates to world") {

    Game game;
    auto& world = game.world();

    float oldZ = world.car().position().z;

    InputState input{};
    input.accelerate = true;

    game.update(0.5f, input);

    REQUIRE(world.car().position().z != oldZ);
}


TEST_CASE("Game reset resets world") {

    Game game;
    auto& world = game.world();

    InputState input{};
    input.accelerate = true;
    game.update(1.f, input);

    float movedZ = world.car().position().z;

    game.reset();

    REQUIRE(world.car().position().z != movedZ);
}

