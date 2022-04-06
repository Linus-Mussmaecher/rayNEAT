//
// Created by Linus on 19.03.2022.
//


#include "rayNEAT.h"

Network::Network(NeatInstance *neatInstance) :  neat_instance(neatInstance), fitness(0) {
    for (int i = 0; i < neat_instance->input_count + neat_instance->output_count; i++) {
        node_values[i] = 0.f;
    }
    for (int i = 0; i < neat_instance->input_count; i++) {
        /*for (int j = input_count; j < input_count + output_count; j++) {
            addConnection(neatInstance, i, j, 1.f);
        }*/
        addConnection(i, GetRandomValue(neat_instance->input_count,
                                        neat_instance->input_count + neat_instance->output_count - 1), 1.f);
    }
}

Network::Network(NeatInstance *neatInstance, const string &line) :  neat_instance(neatInstance) {
    vector<string> datapoints = split(line, "||");
    //fitness
    fitness = std::stof(datapoints[0]);
    //nodes
    vector<string> node_data = split(datapoints[1], ";");
    for (string &nd: node_data) {
        node_values[std::stoi(nd)] = 0.f;
    }
    //connections
    vector<string> connection_data = split(datapoints[2], ";");
    for (string &cd: connection_data) {
        vector<string> connection_datapoints = split(cd, "|");
        connections.push_back({
                                      neatInstance->connection_genes[std::stoi(connection_datapoints[0])],
                                      std::stoi(connection_datapoints[1]) != 0,
                                      std::stof(connection_datapoints[2])

                              });
    }
}


void Network::mutate() {
    if (rnd_f(0.f, 1.f) < neat_instance->probability_mutate_weight) {
        mutateWeights();
    }
    if (rnd_f(0.f, 1.f) < neat_instance->probability_mutate_node) {
        mutateAddNode();
    }
    if (rnd_f(0.f, 1.f) < neat_instance->probability_mutate_link) {
        mutateAddConnection();
    }
}


void Network::mutateWeights() {
    for (Connection &c: connections) {
        if (rnd_f(0.f, 1.f) < neat_instance->probability_mutate_weight_pertube) {
            //slightly pertube weight
            c.weight = std::clamp(c.weight + rnd_f(- neat_instance->mutate_weight_pertube_strength, neat_instance->mutate_weight_pertube_strength), -2.f, 2.f);
        } else {
            //completely randomize weight
            c.weight = rnd_f(-2.f, 2.f);
        }
    }
}


void Network::mutateAddNode() {
    //select a random connection to split with a node
    int split_index = GetRandomValue(0, int(connections.size() - 1));
    Connection split = connections[split_index];
    //get new node id and inform the instance that a new node has been added
    node_id newNode = addNewNode(neat_instance);
    //disable the old connection
    connections[split_index].enabled = false;
    //add new connections from old start to new node to old end
    addConnection(split.gene.start, newNode, split.weight);
    addConnection(newNode, split.gene.end, 1.f);
}

node_id Network::addNewNode(NeatInstance *neatInstance) {
    node_id n = 0;
    while (n < neatInstance->node_count && neatInstance->used_nodes[n]) n++;
    if (n == neatInstance->node_count) {
        //a new global node is required
        neatInstance->node_count++;
        neatInstance->used_nodes.push_back(true);
    } else {
        //an unused node has been found in the global node archive
        neatInstance->used_nodes[n] = true;
    }
    node_values[n] = 0.f;
    return n;
}


void Network::mutateAddConnection() {
    //create a list of all used node_ids
    list<node_id> start_candidates;
    std::transform(node_values.begin(), node_values.end(), std::back_inserter(start_candidates),
                   [](auto &pair) { return pair.first; });
    //a new connection may not start at an output node
    start_candidates.remove_if([&](node_id n) {
        return n >= neat_instance->input_count && n < neat_instance->input_count + neat_instance->output_count;
    });

    list<node_id> end_candidates;
    std::transform(node_values.begin(), node_values.end(), std::back_inserter(end_candidates),
                   [](auto &pair) { return pair.first; });
    //a new connection may not end at an input node
    end_candidates.remove_if([&](node_id n) { return n < neat_instance->input_count; });

    bool found = false;
    node_id start;
    node_id end;

    //search for a non-existing connection
    while (!found && !start_candidates.empty()) {
        //randomly select a start node from the remaining candidates
        start = *std::next(start_candidates.begin(), GetRandomValue(0, int(start_candidates.size() - 1)));
        //create a copy of the end_candidates list. From that one, remove all nodes that are already the end of a connection starting at start
        auto end_candidates_temp = end_candidates;
        for (Connection &c: connections) {
            if (c.gene.start == start) {
                end_candidates_temp.remove(c.gene.end);
            }
        }
        //if there are candidates remaining, select a random one. Otherwise, remove that start node from the list and begin anew
        if (!end_candidates_temp.empty()) {
            found = true;
            end = *std::next(end_candidates_temp.begin(), GetRandomValue(0, int(end_candidates_temp.size() - 1)));
        } else {
            start_candidates.remove(start);
        }
    }
    //if a connection was possible, add it. Otherwise, this network is already fully connected
    if (found) {
        addConnection(start, end, rnd_f(-2.f, 2.f));
    }
}

void Network::addConnection(node_id start, node_id end, float weight) {
    connection_id newID = neat_instance->connection_genes.size();
    //search the global innovation list for a fitting gene
    for (Connection_Gene &cg: neat_instance->connection_genes) {
        if (cg.start == start && cg.end == end) {
            newID = cg.innovation;
        }
    }
    //add the new connection to the list
    connections.push_back({
                                  {newID, start, end},
                                  true, weight
                          });
    //now check if the global innovation list already knows this connection
    if (newID == neat_instance->connection_genes.size()) {
        //nothing was found -> add new innovation to the global list
        neat_instance->connection_genes.push_back({newID, start, end});
    } else {
        //the connection already existed -> sort connnection list to ensure correct ordering
        std::sort(connections.begin(), connections.end(),
                  [](Connection &a, Connection &b) { return a.gene.innovation < b.gene.innovation; });
    }
    //make sure the start & end node of this connection are known
    node_values.try_emplace(start, 0.f);
    node_values.try_emplace(end, 0.f);
}


Network Network::reproduce(Network mother, Network father) {
    Network child(mother.neat_instance);
    child.connections.clear();
    int i = 0;
    int j = 0;
    while (i < mother.connections.size() || j < father.connections.size()) {

        if (i < mother.connections.size() && j < father.connections.size() &&
            mother.connections[i].gene == father.connections[j].gene) {

            //both parents contain the gene
            Connection newConnection = GetRandomValue(0, 1) == 0 ? mother.connections[i] : father.connections[j];
            //if both parents have the gene enabled, the child has it enabled. Otherwise, it is disabled with a 75% chance
            newConnection.enabled =
                    mother.connections[i].enabled && father.connections[j].enabled || (GetRandomValue(1, 100) > 75);
            child.addInheritedConnection(newConnection);
            i++;
            j++;

        } else if (j >= father.connections.size() ||
                   (i < mother.connections.size()
                    && mother.connections[i].gene < father.connections[j].gene)) {
            //father has iterated to the end, so we are in mothers excess genes
            //OR mother is still viable, father is still viable (we have not short-circuited the first expression) and mother has the lower innovation number
            // -> we are in mothers disjoint genes

            //excess genes in mother OR disjoint gens in mother are taken if mother fitness >= father fitness
            if (mother.fitness >= father.fitness) {
                child.addInheritedConnection(mother.connections[i]);
            }
            i++;

        } else if (i >= mother.connections.size() ||
                   (j < father.connections.size() &&
                    mother.connections[i].gene > father.connections[j].gene)) {

            //excess genes in father OR disjoint genes in father are taken if father fitness >= mother fitness
            if (mother.fitness <= father.fitness) {
                child.addInheritedConnection(father.connections[j]);
            }
            j++;

        }

    }

    return child;
}


void Network::addInheritedConnection(Connection c) {
    connections.push_back(c);
    //make sure the nodes are registered
    node_values.try_emplace(c.gene.start, 0.f);
    node_values.try_emplace(c.gene.end, 0.f);
}

void Network::calculateNodeValue(node_id node) {
    //calculate the weighted sum of predecessor nodes and apply activation function
    float weight_sum = 0.f;
    int i = 0;
    for (const Connection &c: connections) {
        if (c.gene.end == node && c.enabled) {
            weight_sum += node_values[c.gene.start] * c.weight;
            i++;
        }
    }
    node_values[node] = neat_instance->activation_function(weight_sum);
}

float Network::getFitness() const {
    return fitness;
}

void Network::setFitness(float fitness_s) {
    Network::fitness = fitness_s;
}


vector<float> Network::calculate(vector<float> inputs){
    //step 1 : set inputs
    for (int i = 0; i < neat_instance->input_count; i++) {
        node_values[i] = i < inputs.size()  ? inputs[i] : 0.f;
    }

    //step 2: propagate values through the network
    for (auto &[id, value]: node_values) {
        if (id >= neat_instance->input_count + neat_instance->output_count) {
            calculateNodeValue(id);
        }
    }

    //step 3: caluclate value of output nodes (positioned at the start of the network) and add to result vector
    vector<float> res;
    for (int i = neat_instance->input_count; i < neat_instance->input_count + neat_instance->output_count; i++) {
        calculateNodeValue(i);
        res.push_back(node_values[i]);
    }

    return res;
}

void Network::print() const {
    std::cout << "Nodes: ";
    for (auto &[id, value]: node_values) {
        std::cout << id << "  ";
    }
    std::cout << "\nConnections:\n";
    for (const Connection &c: connections) {
        std::cout << "\t" << (c.enabled ? "" : "[");
        std::cout << c.gene.innovation << ": " << c.gene.start << " -> " << c.gene.end << ": " << c.weight;
        std::cout << (c.enabled ? "" : "]") << "\n";
    }
}

float Network::getCompatibilityDistance(Network a, Network b) {
    float N = float(std::max(a.connections.size(), b.connections.size()));
    //if (N < 20) N = 1; //Stanley suggests this, and a threshhold of 3.0. -> but then once a genome reaches size 20 it is considered similar to everything else, reducing species size to 1
    float E = 0; //number of eccess genes
    float D = 0; //number of disjoint genes
    float M = 0; //number of matching genes
    float W = 0; //sum of weight differences of matching genes

    int i = 0;
    int j = 0;
    while (i < a.connections.size() || j < b.connections.size()) {
        if (i < a.connections.size() && j < b.connections.size() &&
            a.connections[i].gene == b.connections[j].gene) {
            //a matching gene has been found
            M++;
            W += abs(a.connections[i].weight - b.connections[j].weight);
            i++;
            j++;

        } else if (j >= b.connections.size()) {
            //excess gene of a
            E++;
            i++;
        } else if (i < a.connections.size() && a.connections[i].gene.innovation < b.connections[j].gene.innovation) {
            //disjoint gene of a
            D++;
            i++;
        } else if (i >= a.connections.size()) {
            //excess gene of b
            E++;
            j++;
        } else if (j < b.connections.size() && a.connections[i].gene.innovation > b.connections[j].gene.innovation) {
            //disjoint gene of b
            D++;
            j++;
        }
    }
    if(N == 0) N = 1;
    if(M == 0) M = 1;
    return a.neat_instance->c1 * E / N + a.neat_instance->c2 * D / N + a.neat_instance->c3 * W / M;
}

const vector<Connection> &Network::getConnections() const {
    return connections;
}

const map<node_id, float> &Network::getNodeValues() const {
    return node_values;
}

string Network::toString() const {
    std::ostringstream res;
    res << fitness;
    res << "||";
    for (auto &[id, value]: node_values) {
        res << id << ";";
    }
    res << "||";
    for (const Connection &c: connections) {
        res << c.gene.innovation << "|" << c.enabled << "|" << c.weight << ";";
    }
    return res.str();
}


float sigmoid(float x) {
    return 1.f / (1.f + expf(-4.9f * x));
}


float rnd_f(float lo, float hi) {
    return lo + static_cast <float> (GetRandomValue(0, RAND_MAX)) / (static_cast <float> (RAND_MAX / (hi - lo)));
}