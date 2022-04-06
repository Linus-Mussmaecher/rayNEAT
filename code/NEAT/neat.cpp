//
// Created by Linus on 20.03.2022.
//


#include "rayNEAT.h"


NeatInstance::NeatInstance(unsigned short inputCount, unsigned short outputCount, unsigned int population)
        : input_count(inputCount), output_count(outputCount), population(population), node_count(0),
          repetitions(5),
          generation_number(0), generation_target(100),
          speciation_threshold(1.f), folderpath() {

    //initialization:
    //prepare node list
    node_count = input_count + output_count;
    used_nodes.clear();
    used_nodes.resize(node_count, true);
    //prepare connection list
    connection_genes.clear();
    //prepare network list
    networks.clear();
    networks.reserve(population);
    for (int i = 0; i < population; i++) {
        networks.emplace_back(this);
    }
    //push a single first species that is represented by the first network
    species.clear();
    species.push_back({networks[0]});
}



NeatInstance::NeatInstance(const string &file) {
    std::ifstream savefile;
    savefile.open(file);
    string line;
    //parameters
    std::getline(savefile, line);
    input_count = std::stoi(line);
    std::getline(savefile, line);
    output_count = std::stoi(line);
    std::getline(savefile, line);
    repetitions = std::stoi(line);
    std::getline(savefile, line);
    generation_number = std::stoi(line);
    std::getline(savefile, line);
    generation_target = std::stoi(line);
    std::getline(savefile, line);
    population = std::stoi(line);
    std::getline(savefile, line);
    speciation_threshold = std::stof(line);
    std::getline(savefile, line);
    node_count = std::stoi(line);
    //node data is not loaded as it is not saved. Instead, we just initialize an used_node vector of sufficient size
    used_nodes.resize(node_count, true); //pretend all nodes are used
    //load connection data
    std::getline(savefile, line);
    vector<string> connection_data = split(line, ";");
    for (string &cd: connection_data) {
        vector<string> datapoints = split(cd, "|");
        connection_genes.push_back(
                {
                        static_cast<connection_id>(std::stoi(datapoints[0])),
                        static_cast<connection_id>(std::stoi(datapoints[1])),
                        static_cast<connection_id>(std::stoi(datapoints[2]))
                });
    }

    networks.reserve(population);
    for(int i = 0; i < population; i++){
        std::getline(savefile, line);
        networks.emplace_back(this, line);
    }

    //push a single first species that is represented by the first network. old species are not saved
    species.clear();
    species.push_back({networks[0]});


    savefile.close();

}

void NeatInstance::runNeat(int (*evalNetwork)(Network)) {
    runNeatHelper([evalNetwork, this]() {
        for (Network &n: networks) {
            int f = 0;
            for (int i = 0; i < repetitions; i++) {
                f += evalNetwork(n);
            }
            n.setFitness( std::max(float(f) / float(repetitions), 1.f));
        }
    });
}


void NeatInstance::runNeat(pair<int, int> (*competeNetworks)(Network, Network)) {
    runNeatHelper([competeNetworks, this]() {
        vector<int> f;
        f.resize(networks.size());
        for (int i = 0; i < networks.size(); i++) {
            for (int j = i + 1; j < networks.size(); j++) {
                //std::cout << i << " vs. " << j << "\n";
                for (int r = 0; r < repetitions; r++) {
                    auto[n1_fitness, n2_fitness]= competeNetworks(networks[i], networks[j]);
                    auto[n1_fitnessb, n2_fitnessb]= competeNetworks(networks[j], networks[i]);
                    f[i] += n1_fitness + n1_fitnessb;
                    f[j] += n2_fitness + n2_fitnessb;
                }
            }
            //this network has fought all other networks, so we can now calculate its total fitness
            networks[i].setFitness(float(f[i]) / float(repetitions));
        }
    });
}

void NeatInstance::runNeatHelper(const std::function<void()> &evalNetworks) {


    //algorithm
    while (generation_number++ < generation_target) {

        //step 1: calculate fitness for every network
        evalNetworks();

        //step 2: sort by descending fitness -> all later species will be sorted
        std::sort(networks.begin(), networks.end(),
                  [](Network &n1, Network &n2) { return n1.getFitness() > n2.getFitness(); });

        //step 3: sort into species
        for (Network &network: networks) {
            bool sorted = false;
            for (Species &s: species) {
                if (Network::getCompatibilityDistance(s.representative, network) < speciation_threshold) {
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

        //step 4: remove extinct species
        species.erase(
                std::remove_if(species.begin(), species.end(), [](const Species &s) { return s.networks.empty(); }),
                species.end());
        //output values now, where fitness is not yet shared & networks and species networks are both filled
        print();
        if (generation_number % 10 == 0 && !folderpath.empty()) {
            save();
        }

        //step 5: a random network is chosen as species representative for the next generation
        for (Species &s: species) {
            s.representative = s.networks[GetRandomValue(0, int(s.networks.size() - 1))];
        }

        //step 6: fitness sharing
        float fitness_total = 0.f;
        for (Species &s: species) {
            s.total_fitness = 0.f;
            for (Network &n: s.networks) {
                n.setFitness(std::max(n.getFitness() / float(s.networks.size()),1.f));
                fitness_total += n.getFitness();
                s.total_fitness += n.getFitness();
            }
        }

        //step 7-10
        int elimination_total = int(networks.size()) / 2;
        networks.clear();

        for (Species &s: species) {
            //each species eliminates half of its members (-> atleast 1 member remaining)
            int elimination = int(s.networks.size()) / 2;
            //each species receives offspring according to its fitness
            int offspring = int(s.total_fitness / fitness_total * float(elimination_total));

            //step 7: eliminate weakest members (list is already sorted)
            for (int i = 0; i < elimination && s.networks.size() > 1; i++) {
                s.networks.pop_back();
            }

            //step 8: If species is large enough, immediately push an unmodified champion to the main network list without mutating
            if (s.networks.size() > 5) {
                networks.push_back(s.networks[0]);
                offspring--;
            }

            //step 9: refill with offspring
            int parent_size = int(
                    s.networks.size()); //remember the initial size so that offspring dont mate immediately
            for (int i = 0; i < offspring; i++) {
                if (GetRandomValue(0, 3) == 0 || parent_size == 1) {
                    //mutation without crossover
                    Network n = s.networks[GetRandomValue(0, parent_size - 1)];
                    n.mutate();
                    s.networks.push_back(n);
                } else {
                    //crossover
                    //pick a random father from the non-new networks (not the last one)
                    int father = GetRandomValue(0, parent_size - 2);
                    //pick a random mother with higher index from the non-new networks
                    int mother = GetRandomValue(father + 1, parent_size - 1);
                    Network n = Network::reproduce(s.networks[mother], s.networks[father]);
                    s.networks.push_back(n);
                }
            }

            //step 10: put the networks back in the global list
            for (auto &network: s.networks) {
                network.mutate();
                networks.push_back(network);
            }

            //step 11: clear the networks for the next generation
            s.networks.clear();
        }

        //step 12: refill rounding errors with interspecies reproduction
        while (networks.size() < population) {
            int father = GetRandomValue(0, int(networks.size() - 2));
            int mother = GetRandomValue(father + 1, int(networks.size() - 1));
            Network n = Network::reproduce(networks[mother], networks[father]);
            networks.push_back(n);
        }

        //every 10 algo-steps, re-evaluate which nodes are actively used by networks to allow re-using of smaller indices
        // (limiting size of innovation list, since old connections can also be reused)
        //none of these old connections can currently be in use, else their start & end points would also be in use
        if (generation_number % 10 == 0) {
            std::fill(used_nodes.begin(), used_nodes.end(), false);
            for (Network &n: networks) {
                for (const Connection &c: n.getConnections()) {
                    used_nodes[c.gene.start] = true;
                    used_nodes[c.gene.end] = true;
                }
            }
        }
    }
}

void NeatInstance::print() {
    std::cout << "-------- Generation " << generation_number << " --------\n";

    std::cout << "Best Network:           " << networks[0].getFitness() << "\n";
    if (generation_number % 50 == 0) {
        std::cout << "Best member: \n";
        networks[0].print();
    }

    std::cout << "Total Nodes:            " << used_nodes.size() << "\n";
    std::cout << "Total Connections:      " << connection_genes.size() << "\n";
    std::cout << "Species Count:          " << species.size() << "\n";
    std::cout << "Species Pop:            ";
    for (Species &s: species) {
        std::cout << s.networks.size() << "\t";
    }
    std::cout << "\nSpecies Best:           ";
    for (Species &s: species) {
        std::cout << s.networks[0].getFitness() << "\t";
    }
    std::cout << "\nSpecies Best Node Count:";
    for (Species &s: species) {
        std::cout << s.networks[0].getNodeValues().size() << "\t";
    }
    std::cout << "\nSpecies Best Con Count: ";
    for (Species &s: species) {
        std::cout << s.networks[0].getConnections().size() << "\t";
    }
    std::cout << "\n";
}

void NeatInstance::save() const {
    //create folder if neccessary
    std::filesystem::create_directories(folderpath);
    string filepath;
    filepath.append(folderpath);
    filepath.append("/NEAT_Generation_");
    filepath.append(std::to_string(generation_number));
    filepath.append(".rn");
    //open file
    std::ofstream savefile;
    savefile.open(filepath);
    //save general AI parameters
    savefile
            << input_count << "\n"
            << output_count << "\n"
            << repetitions << "\n"
            << generation_number << "\n"
            << generation_target << "\n"
            << population << "\n"
            << speciation_threshold << "\n"
            << node_count << "\n";
    //used nodes is not saved -> on eventual run it would be updated soon enough
    //save connection gene data -> this allows networks to only save innovation numbers
    for (const Connection_Gene &cg: connection_genes) {
        savefile << cg.innovation << "|" << cg.start << "|" << cg.end << ";";
    }
    savefile << "\n";
    //save networks
    for (const Network &n: networks) {
        savefile << n.toString() << "\n";
    }
    savefile.close();
}

vector<Network> NeatInstance::getNetworksSorted() {
    std::sort(networks.begin(), networks.end(), [](const Network &n1, const Network &n2){return n1.getFitness() < n2.getFitness();});
    return networks;
}

vector<string> split(const string &string_to_split, const string &delimiter) {
    vector<string> res;
    auto start = 0U;
    auto end = string_to_split.find(delimiter);
    while (end != std::string::npos) {
        res.push_back(string_to_split.substr(start, end - start));
        start = end + delimiter.length();
        end = string_to_split.find(delimiter, start);
    }
    res.push_back(string_to_split.substr(start, end));
    //if the string ends with a delimiter, do not include an empty string
    if (res[res.size() - 1].empty()) res.pop_back();
    return res;
}
