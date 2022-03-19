//
// Created by Linus on 19.03.2022.
//


#include <iostream>
#include "rayNEAT.h"

Network::Network(NeatInstance *neatInstance) : input_count(neatInstance->input_count),
                                               output_count(neatInstance->output_count) {
    for (int i = 0; i < input_count + output_count; i++) {
        node_values[i] = 0.f;
    }
    for (int i = 0; i < input_count; i++) {
        for (int j = input_count; j < input_count + output_count; j++) {
            addConnection(neatInstance, i, j, 1.f);
        }
    }
}


void Network::mutateWeights() {
    for (Connection &c: connections) {
        if (GetRandomValue(0, 9) == 0) {
            //in 10% of cases, completely randomize weight
            c.weight = getRandomFloat(-2.f, 2.f);
        } else {
            //otherwise, slightly pertube it
            c.weight = std::clamp(c.weight * getRandomFloat(0.8f, 1.2f), -2.f, 2.f);
        }
    }
}


void Network::mutateAddNode(NeatInstance *neatInstance) {
    //select a random connection to split with a node
    Connection split = connections[GetRandomValue(0, int(connections.size() - 1))];
    //get new node id and inform the instance that a new node has been added
    node_id newNode = neatInstance->node_count++;
    //disable the old connection
    split.enabled = false;
    //add new connections from old start to new node to old end
    addConnection(neatInstance, split.gene.start, newNode, split.weight);
    addConnection(neatInstance, newNode, split.gene.end, 1.f);
}


void Network::mutateAddConnection(NeatInstance *neatInstance) {
    //generate a random start node that may not be an output node
    int start_index = GetRandomValue(0, int(node_values.size() - 1 - output_count));
    if (start_index >= input_count && start_index < input_count + output_count) start_index += output_count;
    node_id  start = std::next(node_values.begin(), start_index)->first;
    //generate a random end node that may not be an input node
    node_id end = std::next(node_values.begin(), GetRandomValue(input_count, int(node_values.size() - 1)))->first;
    //check if the connection is already in the network
    if (std::none_of(connections.begin(), connections.end(), [start, end](const Connection &c) {
        return c.gene.start == start && c.gene.end == end;
    })) {
        addConnection(neatInstance, start, end, getRandomFloat(-2.f, 2.f));
    }
}

void Network::addConnection(NeatInstance *neatInstance, node_id start, node_id end, float weight) {
    connection_id newID = neatInstance->connection_genes.size();
    //search the global innovation list for a fitting gene
    for (Connection_Gene &cg: neatInstance->connection_genes) {
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
    if (newID == neatInstance->connection_genes.size()) {
        //nothing was found -> add new innovation to the global list
        neatInstance->connection_genes.push_back({newID, start, end});
    } else {
        //the connection already existed -> sort connnection list to ensure correct ordering
        std::sort(connections.begin(), connections.end(),
                  [](Connection &a, Connection &b) { return a.gene.innovation < b.gene.innovation; });
    }
    //make sure the start & end node of this connection are known
    node_values.try_emplace(start, 0.f);
    node_values.try_emplace(end, 0.f);
}


Network Network::reproduce(NeatInstance *neatInstance, Network mother, Network father) {
    Network child(neatInstance);
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
                    && mother.connections[i].gene.innovation < father.connections[j].gene.innovation)) {
            //father has iterated to the end, so we are in mothers excess genes
            //OR mother is still viable, father is still viable (we have not short-circuited the first expression) and mother has the lower innovation number -> we are in mothers disjoint genes

            //excess genes in mother OR disjoint gens in mother are taken if mother fitness >= father fitness
            if (mother.fitness >= father.fitness) {
                child.addInheritedConnection(mother.connections[i]);
            }
            i++;

        } else if (i >= mother.connections.size() ||
                   (j < father.connections.size() &&
                    mother.connections[i].gene.innovation > father.connections[j].gene.innovation)) {

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

void Network::calculateOutputs() {
    //step 1: calculate value of all hiden nodes
    for (auto &[id, value]: node_values) {
        if (id >= input_count + output_count) {
            calculateNodeValue(id);
        }
    }
    //step 2: caluclate value of output nodes (positioned at the start of the network)
    for (int i = input_count; i < input_count + output_count; i++) {
        calculateNodeValue(i);
    }
}

void Network::calculateNodeValue(node_id node) {
    //calculate the weighted sum of predecessor nodes and apply sigmoid function
    float weight_sum = 0.f;
    int i = 0;
    for (const Connection &c: connections) {
        if (c.gene.end == node && c.enabled) {
            weight_sum += node_values[c.gene.start] * c.weight;
            i++;
        }
    }
    node_values[node] = sigmoid(weight_sum);
}

unsigned int Network::getFitness() const {
    return fitness;
}

void Network::setFitness(unsigned int fitness_p) {
    Network::fitness = fitness_p;
}

void Network::setInputs(vector<float> inputs) {
    for (int i = 0; i < input_count; i++) {
        node_values[i] = i < inputs.size() ? inputs[i] : 0.f;
    }
}

vector<float> Network::getOutputs() {
    vector<float> res;
    for (int i = input_count; i < output_count + input_count; i++) {
        res.push_back(node_values[i]);
    }
    return res;
}

void Network::print() const {
    std::cout << "Nodes: ";
    for (auto &[id, value]: node_values) {
        std::cout << id << ", ";
    }
    std::cout << "\nConnections:\n";
    for (const Connection &c: connections) {
        std::cout << "\t" << c.gene.innovation << ": " << c.gene.start << " -> " << c.gene.end << ": " << c.weight
                  << "\n";
    }
}


float sigmoid(float x) {
    return 1.f / (1.f + expf(-4.9f * x));
}


float getRandomFloat(float lo, float hi) {
    return lo + static_cast <float> (GetRandomValue(0, RAND_MAX)) / (static_cast <float> (RAND_MAX / (hi - lo)));
}