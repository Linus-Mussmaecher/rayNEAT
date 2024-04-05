#include <iostream>
#include "NEAT/rayNEAT.h"

//scenarios
#include "scenarios/XOR/XOR.h"
#include "scenarios/snake/snake.h"

int main() {
    SetRandomSeed(time(nullptr));

    //evolve_xor();

    // evolve_snake();

    visualize_snake("./snake_0/snake_0_g110_f39.neat");

}
