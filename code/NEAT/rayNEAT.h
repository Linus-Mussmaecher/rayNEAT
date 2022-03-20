//
// Created by Linus on 18.03.2022.
//

#ifndef RAYNEAT_RAYNEAT_H
#define RAYNEAT_RAYNEAT_H

#include "raylib.h"
#include "raymath.h"

#include <cmath>
#include "vector"
#include "list"
#include "array"
#include "map"
#include "string"
#include <iostream>


using std::vector, std::array, std::map, std::string, std::list, std::pair;

//forward declarations
struct NeatInstance;

typedef unsigned int node_id;
typedef unsigned int connection_id;

struct Connection_Gene {
    connection_id innovation;
    node_id start;
    node_id end;
};

bool operator==(Connection_Gene a, Connection_Gene b);

struct Connection {
    Connection_Gene gene;
    bool enabled;
    float weight;
};

class Network {
public:
    Network(NeatInstance* neatInstance);

    //------------------------ mutations
    //randomly mutates the weights in this network
    void mutateWeights();

    //adds a new node into the network
    void mutateAddNode(NeatInstance *neatInstance);

    //adds a completely new link into the network. Registers this link to the global innovation list
    void mutateAddConnection(NeatInstance *neatInstance);

    //adds a new connection to the network and if neccessary registers it with the global innovation list
    void addConnection(NeatInstance *neatInstance, node_id start, node_id end, float weight);

    //------------------------ reproduction
    //creates a child network as combination of mother and father network
    static Network reproduce(NeatInstance *neatInstance, Network mother, Network father);

    //adds a connection to a network INHERITED from a parent. This does assume the connection is already registered globally
    void addInheritedConnection(Connection c);

    //------------------------ calculation
    //calculates the output values of the network after the input values have been set
    void calculateOutputs();

    //calculates the value of a single node based on its predecessors in the network
    void calculateNodeValue(node_id node);

    //------------------------ setters & getters
    [[nodiscard]] unsigned int getFitness() const;

    void setFitness(unsigned int fitness);

    void setInputs(vector<float> inputs);

    vector<float> getOutputs();

    void print() const;

    static float getCompatibilityDistance(Network a, Network b);

private:
    //vector of all connections in this network
    vector<Connection> connections;
    //values of the nodes in this network. first input_count are reserved for input nodes, next output_count for output nodes, rest is hidden nodes
    map<node_id, float> node_values;
    //the last calculated fitness value of this network
    unsigned int fitness = 0;
    //number of input nodes
    const unsigned short input_count;
    //number of output nodes
    const unsigned short output_count;
};

struct Generation {
    unsigned int generation_number;
    vector<Network> networks;

    //species
};

struct NeatInstance {
    const unsigned short input_count;
    const unsigned short output_count;
    Generation current;
    unsigned int node_count = 0;
    vector<Connection_Gene> connection_genes;
};

float sigmoid(float x);

float getRandomFloat(float lo, float hi);


#endif //RAYNEAT_RAYNEAT_H
