//
// Created by Linus on 20.03.2022.
//


#include "rayNEAT.h"


Neat_Instance::Neat_Instance(unsigned short input_count, unsigned short output_count, unsigned int population)
        : input_count(input_count), output_count(output_count), population(population),
          generation_number(0),
          folderpath() {

    //initialization:
    //prepare node list
    for (int i = 0; i < input_count; i++) {
        node_genes.push_back({node_id(i), 0.f, float(i + 1) / float(input_count + 1), true});
    }
    for (int i = 0; i < output_count; i++) {
        node_genes.push_back({node_id(i + input_count),1.f, float(i + 1) / float(output_count + 1), true});
    }
    //prepare network list
    networks.reserve(population);
    for (int i = 0; i < population; i++) {
        networks.emplace_back(this);
    }
}


Neat_Instance::Neat_Instance(const string &file) {
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
    //load node data
    std::getline(savefile, line);
    vector<string> node_data = split(line, ";");
    for (string &nd: node_data) {
        vector<string> datapoints = split(nd, "|");
        node_genes.push_back(
                {
                        static_cast<node_id>(std::stoi(datapoints[0])),
                        std::stof(datapoints[1]),
                        std::stof(datapoints[2]),
                        true
                });
    }
    //load connection data
    std::getline(savefile, line);
    vector<string> connection_data = split(line, ";");
    for (string &cd: connection_data) {
        vector<string> datapoints = split(cd, "|");
        connection_genes.insert(
                {
                        static_cast<connection_id>(std::stoi(datapoints[0])),
                        static_cast<connection_id>(std::stoi(datapoints[1])),
                        static_cast<connection_id>(std::stoi(datapoints[2]))
                });
    }

    networks.reserve(population);
    for (int i = 0; i < population; i++) {
        std::getline(savefile, line);
        networks.emplace_back(this, line);
    }

    savefile.close();

}

void Neat_Instance::run_neat(int (*evalNetwork)(Network)) {
    run_neat_helper([evalNetwork, this]() {
        for (Network &n: networks) {
            int f = 0;
            for (int i = 0; i < repetitions; i++) {
                f += evalNetwork(n);
            }
            n.setFitness(std::max(float(f) / float(repetitions), 1.f));
        }
    });
}


void Neat_Instance::run_neat(pair<int, int> (*compete_networks)(Network, Network)) {
    run_neat_helper([compete_networks, this]() {
        vector<int> f;
        f.resize(networks.size());
        for (int i = 0; i < networks.size(); i++) {
            for (int j = i + 1; j < networks.size(); j++) {
                for (int r = 0; r < repetitions; r++) {
                    auto[n1_fitness, n2_fitness] = compete_networks(networks[i], networks[j]);
                    auto[n1_fitnessb, n2_fitnessb] = compete_networks(networks[j], networks[i]);
                    f[i] += n1_fitness + n1_fitnessb;
                    f[j] += n2_fitness + n2_fitnessb;
                }
            }
            //this network has fought all other networks (except itself), so we can now calculate its total fitness
            networks[i].setFitness(float(f[i]) / float(repetitions));
        }
    });
}

void Neat_Instance::assign_networks_to_species() {
    //clear old species list
    for (Species &s: species) {
        s.networks.clear();
    }

    //assign each species to a network
    for (Network &network: networks) {

        //first, search for a species this network fits in
        auto fitting_species = std::find_if(
                species.begin(), species.end(),
                [&](const Species &s) {
                    return Network::get_compatibility_distance(s.representative, network) < speciation_threshold;
                }
        );

        //if no fitting species was found, create new one with this network as its representative
        if (fitting_species == species.end()) {
            species.push_back({network});
            fitting_species = std::prev(species.end(), 1);
        }

        //finally, add the network to the found/created species
        fitting_species->networks.push_back(network);
    }


    //remove extinct species
    species.remove_if([](const Species &s) { return s.networks.empty(); });

    //TODO: Remove species that haven't innovated in x generations
}

void Neat_Instance::run_neat_helper(const std::function<void()> &evalNetworks) {

    //prime the pump by evaluating, sorting and speciating the initial networks
    evalNetworks();

    std::sort(networks.begin(), networks.end(), &sort_by_fitness_desc);

    assign_networks_to_species();

    while (generation_number++ < generation_target) {


        //the best network is chosen as species representative for the generation about to be calculated
        for (Species &s: species) {
            s.representative = s.networks[0];
        }


        //fitness sharing to assign each species an evolutionary value
        float fitness_total = 0.f;
        for (Species &s: species) {
            s.total_fitness = 0.f;
            for (Network &n: s.networks) {
                fitness_total += n.getFitness() / float(s.networks.size());
                s.total_fitness += n.getFitness() / float(s.networks.size());
            }
        }

        //calculate the total number of networks to be eliminated
        int elimination_total = std::accumulate(
                species.begin(), species.end(), 0,
                [this](int sum, const Species &s) {
                    return sum + int(float(s.networks.size()) * elimination_percentage);
                }
        );

        //clear the network list so the next generation may be added to it
        networks.clear();

        for (Species &s: species) {
            //each species eliminates half of its members (-> atleast 1 member remaining)
            int elimination = int(float(s.networks.size()) * elimination_percentage);
            //each species receives offspring according to its fitness
            int offspring = int(s.total_fitness / fitness_total * float(elimination_total));
            //TODO: Stanley p.13 mentions: "The entire population is then replaced by the offspring of the remaining organisms in each species"

            //eliminate weakest members (list is already sorted)
            s.networks.erase(s.networks.end() - elimination, s.networks.end());

            //if species is large enough, immediately push an unmodified champion to the main network list without mutating
            if (s.networks.size() > 2) {
                networks.push_back(s.networks[0]);
                offspring--;
            }

            //refill with offspring
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

            //put the networks back in the global list
            for (auto &network: s.networks) {
                network.mutate();
                networks.push_back(network);
            }
        }

        //TODO: Stanley p.13: In rare cases when the fitness of the entire population does not improve for more than 20 generations only the top two species are allowed to reproduce, refocusing the search into the most promising spaces

        //refill rounding errors with interspecies reproduction
        while (networks.size() < population) {
            int father = GetRandomValue(0, int(networks.size() - 2));
            int mother = GetRandomValue(father + 1, int(networks.size() - 1));
            Network n = Network::reproduce(networks[mother], networks[father]);
            networks.push_back(n);
        }


        //re-calculate nodes in use

        //every 10 algo-steps, re-evaluate which nodes are actively used by networks to allow re-using of smaller indices
        // (limiting size of innovation list, since old connections can also be reused)
        //none of these old connections can currently be in use, else their start & end points would also be in use
        if (generation_number % 10 == 0) {
            for (Node_Gene &n: node_genes) {
                n.used = false;
            }
            for (Network &n: networks) {
                for (const auto &[id, c]: n.getConnections()) {
                    node_genes[c.gene.start].used = true;
                    node_genes[c.gene.end].used = true;
                }
            }
        }

        //calculate fitness for every network
        evalNetworks();

        //sort by descending fitness -> all later species will be sorted
        std::sort(networks.begin(), networks.end(), &sort_by_fitness_desc);

        //speciate networks
        assign_networks_to_species();

        //save values
        if (generation_number % 10 == 0 && !folderpath.empty()) {
            save();
        }
        print();
    }
}

void Neat_Instance::print() {
    std::cout << "-------- Generation " << generation_number << " --------\n";

    std::cout << "Best Network:           " << networks[0].getFitness() << "\n";
    std::cout << "Total Nodes:            " << node_genes.size() << "\n";
    std::cout << "Total Connections:      " << connection_genes.size() << "\n";
    std::cout << "Species Count:          " << species.size() << "\n";
    std::cout << "Species Information:\n";
    printf("+-----+--------+-----+-----+\n");
    printf("| Pop |  Best  | BNC | BCC |\n");
    printf("+-----+--------+-----+-----+\n");
    for (Species &s: species) {
        printf(
                "| %3llu | %6.1f | %3llu | %3llu |\n",
                s.networks.size(),
                s.networks[0].getFitness(),
                s.networks[0].getNodes().size(),
                s.networks[0].getConnections().size()
        );
    }
    printf("+-----+--------+-----+-----+\n");
}

void Neat_Instance::save() const {
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
            << speciation_threshold << "\n";
    //used nodes is not saved -> on eventual run it would be updated soon enough
    //save connection gene data -> this allows networks to only save innovation numbers
    for (const Node_Gene &ng: node_genes) {
        savefile << ng.id << "|" << ng.x << "|" << ng.y << ";";
    }
    savefile << "\n";
    for (const Connection_Gene &cg: connection_genes) {
        savefile << cg.id << "|" << cg.start << "|" << cg.end << ";";
    }
    savefile << "\n";
    //save networks
    for (const Network &n: networks) {
        savefile << n.to_string() << "\n";
    }
    savefile.close();
}

vector<Network> Neat_Instance::get_networks_sorted() {
    std::sort(networks.begin(), networks.end(),
              [](const Network &n1, const Network &n2) { return n1.getFitness() > n2.getFitness(); });
    return networks;
}

Node_Gene Neat_Instance::request_node_gene(Connection_Gene split) {
    //TODO: Return nodes already used by other networks
    //check if there is an unused node gene
    auto unused_node = std::find_if(node_genes.begin(), node_genes.end(), [](const Node_Gene &ng) { return !ng.used; });
    node_id id;
    //select either an unused node or create a new one
    if (unused_node == node_genes.end()) {
        id = node_id(node_genes.size());
        node_genes.push_back({id, 0.f, 0.f, true});
    } else {
        id = unused_node->id;
    }
    //set the (new) value for the (new) node_gene
    node_genes[id].x = (node_genes[split.start].x + node_genes[split.end].x) / 2.f;
    node_genes[id].y = (node_genes[split.start].y + node_genes[split.end].y) / 2.f + rnd_f(0.f, 0.1f);
    node_genes[id].used = true;
    //return the result
    return node_genes[id];
}

Node_Gene Neat_Instance::request_node_gene(node_id id) {
    return node_genes[id];
}


Connection_Gene Neat_Instance::request_connection_gene(node_id start, node_id end) {
    Connection_Gene ng = {connection_id(connection_genes.size()), start, end};
    if (!connection_genes.contains(ng)) {
        connection_genes.insert(ng);
    }
    return *connection_genes.find(ng);
}

Connection_Gene Neat_Instance::request_connection_gene(connection_id id) {
    return *std::find_if(
            connection_genes.begin(), connection_genes.end(),
            [id](const Connection_Gene &cg) { return cg.id == id; }
    );
}

bool Neat_Instance::sort_by_fitness_desc(const Network &n1, const Network &n2) {
    return n1.getFitness() > n2.getFitness();
}
