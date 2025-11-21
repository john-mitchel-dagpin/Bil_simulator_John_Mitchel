
#ifndef BIL_SIMULATOR_JOHN_MITCHEL_OBSTACLE_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_OBSTACLE_HPP

#pragma once
#include "GameObject.hpp"

class Obstacle : public GameObject {
public:
    Obstacle(float x, float z, float halfW, float halfL);
    void onCarOverlap(Car& car) override;
};


#endif //BIL_SIMULATOR_JOHN_MITCHEL_OBSTACLE_HPP