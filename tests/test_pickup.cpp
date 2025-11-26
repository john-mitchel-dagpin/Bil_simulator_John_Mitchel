#include <catch2/catch_test_macros.hpp>
#include "Pickup.hpp"
#include "Car.hpp"

TEST_CASE("Pickup is active when created") {

    Pickup p(Pickup::Type::SpeedBoost, 0.f, 0.f);

    REQUIRE(p.isActive());
}

TEST_CASE("Pickup deactivates after car overlaps") {

    Car car;
    Pickup p(Pickup::Type::SpeedBoost, 0.f, 0.f);

    // Før overlapping
    REQUIRE(p.isActive());

    // Trigger overlap logic
    p.onCarOverlap(car);

    // Etter overlap skal pickup være inaktiv
    REQUIRE_FALSE(p.isActive());
}
