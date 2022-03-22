#include <iostream>
#include "NEAT/rayNEAT.h"

int main() {
    SetRandomSeed(clock());

    NeatInstance neatInstance = NeatInstance();
    neatInstance.input_count = 3;
    neatInstance.output_count = 1;
    neatInstance.generation_target = 100;
    neatInstance.population = 100;
    neatInstance.folderpath = "./XOR";
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

    Network best = Network::loadNthBest("./XOR/NEAT_Generation_100.rn");

    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            best.setInputs({float(a), float(b), 1.f});
            best.calculateOutputs();
            std::cout << a << " ^ " << b << " = " << best.getOutputs()[0] << "\n";
        }
    }
}
