//
// Created by Linus on 20.03.2022.
//


#include "rayNEAT.h"

void NeatInstance::initialize(unsigned int population) {
    networks.reserve(population);
    for (int i = 0; i < population; i++) {
        networks.emplace_back(this);
    }
    //push a single first species that is represented by the first network
    species.push_back({networks[0]});
}

void NeatInstance::runNeat(int (*evalNetwork)(Network), int repetitions) {
    runNeatHelper([evalNetwork, repetitions, this]() {
        for (Network &n: networks) {
            int f = 0;
            for (int i = 0; i < repetitions; i++) {
                f += evalNetwork(n);
            }
            n.setFitness(float(f) / float(repetitions));
        }
    }, 500);
}


void NeatInstance::runNeat(pair<int, int> (*competeNetworks)(Network, Network), int repetitions) {
    runNeatHelper([competeNetworks, repetitions, this]() {
        vector<int> f;
        f.reserve(networks.size());
        for (int i = 0; i < networks.size(); i++) {
            for (int j = i + 1; j < networks.size(); j++) {
                for (int r = 0; r < repetitions; r++) {
                    auto[n1_fitness, n2_fitness]= competeNetworks(networks[i], networks[j]);
                    f[i] += n1_fitness;
                    f[j] += n2_fitness;
                }
            }
            //this network has fought all other networks, so we can now calculate its total fitness
            networks[i].setFitness(float(f[i]) / float(repetitions));
        }
    }, 101);
}

void NeatInstance::runNeatHelper(const std::function<void()> &evalNetworks, int generation_target) {
    while (generation_number++ < generation_target) {
        std::cout << "-------- Generation " << generation_number << " --------\n";

        //step 1: calculate fitness for every network
        evalNetworks();

        //step 2: sort by descending fitness -> all later species will be sorted
        std::sort(networks.begin(), networks.end(),
                  [](Network &n1, Network &n2) { return n1.getFitness() > n2.getFitness(); });

        std::cout << "Best Network:           " << networks[0].getFitness() << "\n";
        if (generation_number % 50 == 0) {
            std::cout << "Best member: \n";
            networks[0].print();
        }

        //step 3: sort into species
        for (Network &network: networks) {
            bool sorted = false;
            //std::cout << "Sorting a network of fitness " << network.getFitness() << ".\n";
            for (Species &s: species) {
                //std::cout << "Checking a species with representative of fitness " << s.representative.getFitness() << ".\n";
                if (Network::getCompatibilityDistance(s.representative, network) < speciation_threshold) {
                    //std::cout << "This fits!\n";
                    s.networks.push_back(network);
                    sorted = true;
                    break;
                }
            }
            //a new species is created
            if (!sorted) {
                //std::cout << "Trying to add a new species!\n";
                Species s = {network};
                s.networks.push_back(network);
                species.push_back(s);
            }
        }

        //remove extinct species
        species.erase(
                std::remove_if(species.begin(), species.end(), [](const Species &s) { return s.networks.empty(); }),
                species.end());

        //step 4: a random network is chosen as species representative for the next generation
        for (Species &s: species) {
            s.representative = s.networks[GetRandomValue(0, int(s.networks.size() - 1))];
        }

        std::cout << "Species Count:          " << species.size() << "\n";
        std::cout << "Species Pop:            ";
        for (Species &s: species) {
            std::cout << s.networks.size() << "\t";
        }
        std::cout << "\nSpecies Best:           ";
        for (Species &s: species) {
            std::cout << s.networks[0].getFitness() << "\t";
        }
        std::cout << "\n";

        //step 5: fitness sharing
        float fitness_total = 0.f;
        for (Species &s: species) {
            s.total_fitness = 0.f;
            for (Network &n: s.networks) {
                n.setFitness(n.getFitness() / float(s.networks.size()));
                fitness_total += n.getFitness();
                s.total_fitness += n.getFitness();
            }
        }

        //step 6-8
        int population = int(networks.size());
        int elimination_total = int(networks.size()) / 2;
        networks.clear();

        for (Species &s: species) {
            //each species eliminates half of its members (-> atleast 1 member remaining)
            int elimination = int(s.networks.size()) / 2;
            //each species receives offspring according to its fitness
            int offspring = int(s.total_fitness / fitness_total * float(elimination_total));

            //step 6: eliminate weakest members (list is already sorted)
            for (int i = 0; i < elimination && s.networks.size() > 1; i++) {
                s.networks.pop_back();
            }

            //step 7: refill with offspring
            //step 7.1: If species is large enough, immediately push an unmodified champion to the main network list without mutating
            if(s.networks.size() > 5) {
                networks.push_back(s.networks[0]);
                offspring--;
            }
            //now create the rest of the offspring by mutation and reproduction
            int trimmed_size = int(s.networks.size());
            for (int i = 0; i < offspring; i++) {
                if (GetRandomValue(0, 3) == 0 || trimmed_size == 1) {
                    //mutation without crossover
                    Network n = s.networks[GetRandomValue(0, trimmed_size - 1)];
                    n.mutate(this);
                    s.networks.push_back(n);
                } else {
                    //crossover
                    int father = GetRandomValue(0, trimmed_size -
                                                   2); //pick a random father from the non-new networks (not the last one)
                    int mother = GetRandomValue(father + 1, trimmed_size -
                                                            1); //pick a random mother with higher index from the non-new networks
                    Network n = Network::reproduce(this, s.networks[mother], s.networks[father]);
                    s.networks.push_back(n);
                }
            }

            //step 8: put the networks back in the global list
            for (int i = 0; i < s.networks.size(); i++) {
                s.networks[i].mutate(this);
                networks.push_back(s.networks[i]);
            }

            //step 9: clear the networks for the next generation
            s.networks.clear();
        }

        //step 10: refill rounding errors with interspecies reproduction
        std::cout << "Interspecies Children:  " << population - networks.size() << "\n";
        while (networks.size() < population) {
            int father = GetRandomValue(0, int(networks.size() - 2));
            int mother = GetRandomValue(father + 1, int(networks.size() - 1));
            Network n = Network::reproduce(this, networks[mother], networks[father]);
            networks.push_back(n);
        }
    }
}
