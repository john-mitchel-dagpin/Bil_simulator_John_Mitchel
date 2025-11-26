#include <catch2/catch_test_macros.hpp>
#include "World.hpp"
#include "Car.hpp"

TEST_CASE("World initializes with objects") {
    World w;
    REQUIRE(w.objects().size() > 0);
}

TEST_CASE("World gate states start closed") {
    World w;
    REQUIRE_FALSE(w.gate1IsOpen());
    REQUIRE_FALSE(w.gate2IsOpen());
    REQUIRE_FALSE(w.gate3IsOpen());
}

TEST_CASE("World portal starts inactive") {
    World w;
    REQUIRE_FALSE(w.portalTriggered());
}
