#include <catch2/catch_test_macros.hpp>
#include "Car.hpp"

TEST_CASE("Car moves forward", "[car]") {
    Car c;
    c.position = Vec2(0,0);
    c.speed = 0;
    c.maxSpeed = 10;
    c.update(0.1f, true, false, false, false); // forward for 0.1s

    REQUIRE((c.position.x != 0.f || c.position.y != 0.f));
}

TEST_CASE("Collision detection", "[collision]") {
    Car c;
    c.position = Vec2(0.f,0.f);

    REQUIRE(c.checkCollision(Vec2(0.5f,0.f), 0.6f, 0.6f) == true);
    REQUIRE(c.checkCollision(Vec2(5.f,5.f), 0.6f, 0.6f) == false);
}
