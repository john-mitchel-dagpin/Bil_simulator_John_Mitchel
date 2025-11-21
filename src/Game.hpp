

#ifndef BIL_SIMULATOR_JOHN_MITCHEL_GAME_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_GAME_HPP
#pragma once
#include "World.hpp"

class Game {
public:
    void update(float dt, const InputState& input) {
        world_.update(dt, input);
    }

    void reset() {
        world_.reset();
    }

    World& world() { return world_; }

private:
    World world_;
};

#endif //BIL_SIMULATOR_JOHN_MITCHEL_GAME_HPP