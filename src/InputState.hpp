

#ifndef BIL_SIMULATOR_JOHN_MITCHEL_INPUTSTATE_HPP
#define BIL_SIMULATOR_JOHN_MITCHEL_INPUTSTATE_HPP
#pragma once

struct InputState {
    bool accelerate = false;
    bool brake = false;
    bool turnLeft = false;
    bool turnRight = false;
};


#endif //BIL_SIMULATOR_JOHN_MITCHEL_INPUTSTATE_HPP