#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "Car.hpp"
#include "InputState.hpp"

using Catch::Approx;

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

    input.accelerate = true;
    car.update(1.f, input);
    input.accelerate = false;

    float speedAfterAccel = car.speed();

    car.update(1.f, input);

    REQUIRE(car.speed() < speedAfterAccel);
    REQUIRE(car.speed() >= 0.f);
}

TEST_CASE("Car turns left/right when steering") {
    Car car;
    InputState input{};
    input.accelerate = true;

    float initialRot = car.rotation();

    input.turnLeft = true;
    car.update(1.f, input);

    REQUIRE(car.rotation() != Approx(initialRot));
}
