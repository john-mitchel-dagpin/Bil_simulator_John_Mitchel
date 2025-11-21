#include <catch2/catch_test_macros.hpp>
#include "Car.hpp"

TEST_CASE("Car accelerates when pressing forward") {
    Car car;
    InputState input{};
    input.accelerate = true;

    const float dt = 0.1f;
    car.update(dt, input);

    REQUIRE(car.speed() > 0.f);
}

TEST_CASE("Car slows down due to friction") {
    Car car;
    InputState input{};

    // simulate some acceleration
    input.accelerate = true;
    car.update(1.f, input);
    input.accelerate = false;

    const float speedAfterAccel = car.speed();
    car.update(1.f, input);

    REQUIRE(car.speed() < speedAfterAccel);
    REQUIRE(car.speed() >= 0.f);
}
