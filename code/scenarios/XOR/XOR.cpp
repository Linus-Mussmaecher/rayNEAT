//
// Created by Linus on 22.03.2022.
//

#include "XOR.h"

void testXOR(){

    Neat_Instance neatInstance = Neat_Instance(3, 1, 100);
    //Neat_Instance neatInstance = Neat_Instance("./XOR/NEAT_Generation_250.rn");
    neatInstance.generation_target = 50;
    neatInstance.elimination_percentage = 0.2f;
    neatInstance.folderpath = "./XOR";
    neatInstance.run_neat(&testNetworkXOR);

    Network best = neatInstance.get_networks_sorted()[0];

    best.print();

    std::cout << "Score: " << best.getFitness() << "\n";

    float res = 0;
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            float v = best.calculate({float(a), float(b), 1.f})[0];
            std::cout << a << " ^ " << b << " = " << v << "\n";
            res += abs(v - float(a ^ b));
        }
    }
    std::cout << "Score: " << int((4.f - res) * (4.f - res) * 100) << "\n";
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