//
// Created by Linus on 22.03.2022.
//

#include "XOR.h"

void testXOR(){

    NeatInstance neatInstance = NeatInstance(3,1,100);
    //NeatInstance neatInstance = NeatInstance("./XOR/NEAT_Generation_100.rn");
    neatInstance.generation_target = 150;
    neatInstance.folderpath = "./XOR";
    neatInstance.runNeat(&testNetworkXOR);

    Network best = neatInstance.getNetworksSorted()[0];

    std::cout << "Score: " << best.getFitness() << "\n";

    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            std::cout << a << " ^ " << b << " = " << best.calculate({float(a), float(b), 1.f})[0] << "\n";
        }
    }
}

int testNetworkXOR(Network n){
    float res = 0;
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            res += abs(n.calculate({float(a), float(b), 1.f})[0] - float(a ^ b));
        }
    }
    return int((4.f - res) * (4.f - res) * 100);
}