#include <iostream>
#include "NEAT/rayNEAT.h"

int main() {
    SetRandomSeed(clock());

    NeatInstance neatInstance = NeatInstance(3, 1, 5, 150, 100);
    neatInstance.runNeat([](Network n) {
        float res = 0;
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                n.setInputs({float(a), float(b), 1.f});
                n.calculateOutputs();
                res += abs(n.getOutputs()[0] - float(a ^ b));
            }
        }
        return int((4.f - res) * (4.f - res) * 100);
    });

}
