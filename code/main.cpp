#include <iostream>
#include "NEAT/rayNEAT.h"

//scenarios
#include "scenarios/XOR/XOR.h"

int main() {
    SetRandomSeed(time(nullptr));

    testXOR();
}
