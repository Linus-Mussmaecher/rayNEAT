#include <iostream>
#include "NEAT/rayNEAT.h"

//scenarios
#include "scenarios/XOR/XOR.h"
#include "scenarios/snake/snake.h"

int main() {
    SetRandomSeed(clock());


    testSnake();
    Network n = Network::loadNthBest("./snake/NEAT_Generation_100.rn", 10);
    runSnakeVisual(n, true);
}
