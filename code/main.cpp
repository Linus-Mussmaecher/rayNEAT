#include <iostream>
#include "NEAT/rayNEAT.h"

int main() {
    SetRandomSeed(clock());

    NeatInstance neatInstance = {.input_count = 4, .output_count = 3, .node_count =  7};

    vector<Network> networks;
    networks.reserve(16);
    for (int i = 0; i < 5; i++) {
        networks.emplace_back(&neatInstance);
    }

    int f = 0;
    for (Network &n1: networks) {
        n1.setFitness(f++);
        n1.mutateAddConnection(&neatInstance);
        n1.mutateAddNode(&neatInstance);
        n1.mutateAddConnection(&neatInstance);
        n1.mutateAddConnection(&neatInstance);
        n1.mutateAddNode(&neatInstance);
        n1.mutateAddConnection(&neatInstance);
        n1.mutateAddConnection(&neatInstance);
        n1.mutateAddConnection(&neatInstance);
        n1.mutateWeights();
    }

    networks[3].print();
    networks[4].print();

    Network nextWork = Network::reproduce(&neatInstance, Network::reproduce(&neatInstance, networks[3], networks[4]), Network::reproduce(&neatInstance, networks[1], networks[2]));
    nextWork.print();

    nextWork.setInputs({0.4f, -0.9f, 0.2f, 0.2f});
    nextWork.calculateOutputs();
    std::cout << "Outputs: ";
    for(float op : nextWork.getOutputs()){
        std::cout << op << " ";
    }


    return 0;
}
