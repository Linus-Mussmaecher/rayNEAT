//
// Created by Linus on 22.03.2022.
//

#include "XOR.h"

void testXOR(){

    Neat_Instance neatInstance = Neat_Instance(3, 1, 150);
    //Neat_Instance neatInstance = Neat_Instance("./XOR/NEAT_Generation_100.rn");
    neatInstance.generation_target = 150;
    neatInstance.node_count_exponent = 0.2;
    neatInstance.elimination_percentage = 0.5f;
    neatInstance.folderpath = "./XOR";
    neatInstance.run_neat(&testNetworkXOR);

    InitWindow(800, 600, "Best Network");

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
    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        best.draw({20,20,740,540});
        EndDrawing();
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