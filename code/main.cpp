#include <iostream>
#include "NEAT/rayNEAT.h"

//scenarios
#include "scenarios/XOR/XOR.h"
#include "scenarios/snake/snake.h"

int main() {
    SetRandomSeed(time(nullptr));

    //evolve_snake();

    visualize_snake();

}
