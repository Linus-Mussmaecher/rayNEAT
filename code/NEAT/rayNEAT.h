//
// Created by Linus on 18.03.2022.
//

#ifndef RAYNEAT_RAYNEAT_H
#define RAYNEAT_RAYNEAT_H

#include "raylib.h"
#include "raymath.h"

#include <cmath>
#include <vector>
#include <list>
#include <array>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <algorithm>
#include <sstream>

using std::vector, std::array, std::map, std::string, std::list, std::pair;

//parameters


//forward declarations
class NeatInstance;

typedef unsigned int node_id;
typedef unsigned int connection_id;

struct Connection_Gene {
    connection_id innovation;
    node_id start;
    node_id end;
};

bool operator==(Connection_Gene a, Connection_Gene b);

bool operator<(Connection_Gene a, Connection_Gene b);

bool operator>(Connection_Gene a, Connection_Gene b);

struct Connection {
    Connection_Gene gene;
    bool enabled;
    float weight;
};

class Network {
public:
    explicit Network(NeatInstance *neatInstance);

    //------------------------ mutations
    //randomly performs mutations on this network
    void mutate(NeatInstance *neatInstance);

    //randomly mutates the weights in this network
    void mutateWeights();

    //adds a new node into the network
    void mutateAddNode(NeatInstance *neatInstance);

    //adds a completely new link into the network. Registers this link to the global innovation list
    void mutateAddConnection(NeatInstance *neatInstance);

    //adds a completely new node to the network not yet used by any other network
    node_id addNewNode(NeatInstance *neatInstance);

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
    [[nodiscard]] float getFitness() const;

    void setFitness(float fitness);

    [[nodiscard]] const vector<Connection> &getConnections() const;

    [[nodiscard]] const map<node_id, float> &getNodeValues() const;

    //sets the values of the input nodes (these will internally be scaled to be in the [0,1] intervall)
    void setInputs(vector<float> inputs);

    vector<float> getOutputs();

    //print a human-readable description to the standard output
    void print() const;

    //return a somewhat human-readable and very machine readable string that describes this networks nodes & connections. connections only print their innovation!
    [[nodiscard]] string toString() const;

    static float getCompatibilityDistance(Network a, Network b);

private:
    //vector of all connections in this network
    vector<Connection> connections;
    //values of the nodes in this network. first input_count are reserved for input nodes, next output_count for output nodes, rest is hidden nodes
    map<node_id, float> node_values;
    //the last calculated fitness value of this network
    float fitness = 0;
    //number of input nodes
    unsigned short input_count;
    //number of output nodes
    unsigned short output_count;
};

struct Species {
    Network representative;
    float total_fitness = 0.f;
    vector<Network> networks;
};

class NeatInstance {
public:

    // ------------ General Algorithm parameters ------------
    //Constructor initializing the following parameters
    NeatInstance(unsigned short inputCount, unsigned short outputCount, unsigned int repetitions,
                 unsigned int generationTarget, unsigned int population);

    //number of input nodes each network has
    const unsigned short input_count;
    //number of output nodes each network has
    const unsigned short output_count;
    //how often every single network is evaluated to calculate its fitness each round
    const unsigned int repetitions;
    //how many generations should be simulated
    const unsigned int generation_target;
    //number of total networks
    const unsigned int population;
    //distance threshhold for when two networks are considered to be of the same species
    const float speciation_threshold = 1.f;

    // ------------ Gene archives ------------

    unsigned int node_count;
    vector<bool> used_nodes;
    vector<Connection_Gene> connection_genes;

    // ------------ Execution options ------------

    //performs the NEAT algorithm. Each network's fitness is evaluated with the provided function
    void runNeat(int (*evalNetwork)(Network));
    //performs the NEAT algorithm. Each networks's fitness is evaluated by letting them compete with each other network
    //using the provided function and averaging fitness results
    void runNeat(pair<int, int> (*competeNetworks)(Network, Network));

    // ------------ Printing ------------

    //prints information about all networks to the standard output
    void print();
private:
    //all networks managed by this instance
    vector<Network> networks;
    //the same networks separated into species
    vector<Species> species;
    //the current number of simulated generations
    unsigned int generation_number = 0;

    //performs the NEAT-algorithm on the network list.
    //the passed function must set the fitness values of all the networks in the list.
    //should only be called by the public runNeat functions
    void runNeatHelper(const std::function<void()> &evalNetworks);
};

//a modified sigmoid function
float sigmoid(float x);

//returns a randomly selected float between the two passed values
float getRandomFloat(float lo, float hi);


#endif //RAYNEAT_RAYNEAT_H
