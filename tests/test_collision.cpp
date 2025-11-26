#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "World.hpp"
#include "Pickup.hpp"
#include "Obstacle.hpp"
#include "InputState.hpp"

using Catch::Approx;

TEST_CASE("Car picks up a pickup when colliding") {

    World world;

    // Ensure world has at least one pickup object
    bool foundPickup = false;

    for (auto& obj : world.objects()) {
        if (auto* p = dynamic_cast<Pickup*>(obj.get())) {

            foundPickup = true;

            // Move car onto pickup
            auto b = p->bounds();
            float x = (b.minX + b.maxX) * 0.5f;
            float z = (b.minZ + b.maxZ) * 0.5f;

            world.car().setPosition(x, z);

            InputState input{};
            world.update(0.1f, input);

            REQUIRE(p->isActive() == false);
            break;
        }
    }

    REQUIRE(foundPickup);
}



TEST_CASE("Car colliding with obstacle stops movement") {

    World world;

    Obstacle* obstacle = nullptr;

    for (auto& obj : world.objects()) {
        if (auto* o = dynamic_cast<Obstacle*>(obj.get())) {
            obstacle = o;
            break;
        }
    }

    REQUIRE(obstacle != nullptr);

    // Move car into the obstacle's bounding box
    auto b = obstacle->bounds();
    float cx = (b.minX + b.maxX) * 0.5f;
    float cz = (b.minZ + b.maxZ) * 0.5f;

    world.car().setPosition(cx, cz);

    InputState input{};
    input.accelerate = true;

    world.update(0.1f, input);

    REQUIRE(world.car().speed() <= 0.f);
}



TEST_CASE("Entering portal triggers portal state") {

    World world;

    auto c = world.portalCenter();

    world.car().setPosition(c.x, c.z);

    InputState input{};
    world.update(0.1f, input);

    REQUIRE(world.portalTriggered());
}

