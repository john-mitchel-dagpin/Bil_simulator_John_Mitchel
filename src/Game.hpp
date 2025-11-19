#ifndef BIL_SIMULATOR_JOHN_MITCHEL_GAME_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_GAME_HPP

#pragma once

#include "Car.hpp"
#include "physics.hpp"

#include <threepp/threepp.hpp>
#include <memory>
#include <vector>

using namespace threepp;

struct Pickup {
    ::Vec2 pos;
    float radius;
    bool collected = false;
    enum Type { SPEED, BIGNESS } type;
};

struct Obstacle {
    ::Vec2 pos;
    float radius;
    std::shared_ptr<threepp::Mesh> mesh;
};

class Game {

public:
    Game(Canvas& canvas, GLRenderer& renderer);
    void run();
    void reset();

private:
    Canvas& canvas_;
    GLRenderer& renderer_;

    std::shared_ptr<Scene> scene_;
    std::shared_ptr<PerspectiveCamera> camera_;

    Car car_;

    float carRadius_ = 1.f;
    float carRenderScale_ = 1.f;

    std::shared_ptr<Mesh> carMesh_;
    std::vector<std::shared_ptr<Mesh>> wheels_;

    std::vector<Pickup> pickups_;
    std::vector<std::shared_ptr<Mesh>> pickupMeshes_;

    std::vector<Obstacle> obstacles_;

    void initScene();
    void spawnPickups();
    void handlePickup(Pickup& p, std::shared_ptr<Mesh>& mesh);
};

#endif
