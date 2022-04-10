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
#include <numeric>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <utility>
#include <set>
#include <unordered_set>

using std::vector, std::array, std::map, std::string, std::list, std::pair, std::set, std::unordered_set;

//forward declarations
struct Node_Gene;
struct Node;
struct Connection_Gene;
struct Connection;
class Neat_Instance;
class Network;

/*   +------------------------------------------------------------+
 *   |                                                            |
 *   |                            Node                            |
 *   |                                                            |
 *   +------------------------------------------------------------+
 */

typedef unsigned int node_id;

struct Node_Gene {
    node_id id;
    float x;
    bool used;
};

struct Node {
    Node_Gene gene;
    float y;
    float value;
};

//connection compare as equal if their ids are equal
bool operator==(Node_Gene a, Node_Gene b);

//nodes are sorted based on their id
bool operator<(Node_Gene a, Node_Gene b);

//nodes are sorted based on their id
bool operator>(Node_Gene a, Node_Gene b);

template<>
struct std::hash<Node_Gene>
{
    std::size_t operator()(Node_Gene const& ng) const noexcept
    {
        return ng.id;
    }
};

/*   +------------------------------------------------------------+
 *   |                                                            |
 *   |                        Connection                          |
 *   |                                                            |
 *   +------------------------------------------------------------+
 */


typedef unsigned int connection_id;

struct Connection_Gene {
    connection_id id;
    node_id start;
    node_id end;
};

struct Connection {
    Connection_Gene gene;
    bool enabled;
    float weight;
};

//connection compare as equal if their start- and endpoints are equal
bool operator==(Connection_Gene a, Connection_Gene b);

//connections are sorted based on their id
bool operator<(Connection_Gene a, Connection_Gene b);

//connections are sorted based on their id
bool operator>(Connection_Gene a, Connection_Gene b);

template<>
struct std::hash<Connection_Gene>
{
    std::size_t operator()(Connection_Gene const& cg) const noexcept
    {
        return (cg.start << 16) ^ cg.end;
    }
};

/*   +------------------------------------------------------------+
 *   |                                                            |
 *   |                          Network                           |
 *   |                                                            |
 *   +------------------------------------------------------------+
 */


class Network {
public:
    // ------------ constructors ------------

    explicit Network(Neat_Instance *neatInstance);

    //loads a network based on a neatInstance and a line from a .nt file
    explicit Network(Neat_Instance *neatInstance, const string &line);

    // ------------ mutations ------------

    //randomly performs mutations on this network
    void mutate();

    //randomly mutates the weights in this network
    void mutate_weights();

    //adds a new node into the network
    void mutate_addnode();

    //adds a completely new link into the network. Registers this link to the global innovation list
    void mutate_addconnection();

    //adds a completely new node to the network not yet used by any other network
    node_id add_node(Neat_Instance *neatInstance);

    //adds a new connection to the network and if neccessary registers it with the global innovation list
    void add_connection(node_id start, node_id end, float weight);

    // ------------ reproduction ------------

    //creates a child network as combination of mother and father network
    static Network reproduce(Network mother, Network father);

    //adds a connection to a network INHERITED from a parent. This does assume the connection is already registered globally
    void add_inherited_connection(Connection c);

    // ------------ calculation ------------

    //calculates the value of a single node based on its predecessors in the network
    void calculate_node_value(node_id node);

    //sets the input values of the network, calculates and returns the output values
    vector<float> calculate(vector<float> inputs);

    // ------------ setters & getters ------------

    [[nodiscard]] float getFitness() const;

    void setFitness(float fitness);

    [[nodiscard]] const map<connection_id, Connection> & getConnections() const;

    [[nodiscard]] const map<node_id, float> &getNodeValues() const;

    //print a human-readable description to the standard output
    void print() const;

    //draw a rendering of the network to the raylib. Must already be in drawing mode.
    void draw() const;

    //return a somewhat human-readable and very machine readable string that describes this networks nodes & connections. connections only print their innovation!
    [[nodiscard]] string to_string() const;

    static float get_compatibility_distance(Network a, Network b);

private:
    //map of all connections in this network, mapping their id to the full struct (for sorting + easy access)
    map<connection_id , Connection> connections;
    //values of the nodes in this network. first input_count are reserved for input nodes, next output_count for output nodes, rest is hidden nodes
    map<node_id, float> node_values;
    //the last calculated fitness value of this network
    float fitness;
    //the neatInstace this network belongs to
    Neat_Instance *neat_instance;
};

struct Species {
    Network representative;
    float total_fitness = 0.f;
    vector<Network> networks;
};

/*   +------------------------------------------------------------+
 *   |                                                            |
 *   |                           NEAT                             |
 *   |                                                            |
 *   +------------------------------------------------------------+
 */


//a modified sigmoid function
float sigmoid(float x);

class Neat_Instance {
public:
    // ------------ Constructors ------------

    //initializes a fresh neatInstance with the provided parameteres. non-fixed parameters are public and may be accessed afterwards
    explicit Neat_Instance(unsigned short input_count, unsigned short output_count, unsigned int population);

    //initializes a neatInstance from the provided file, loading its fixed parameters. non-fixed parameters may be adjusted for the continuation
    explicit Neat_Instance(const string &file);

    // ------------ Parameters ------------

    //number of input nodes each network has
    unsigned short input_count;
    //number of output nodes each network has
    unsigned short output_count;
    //number of total networks
    unsigned int population;

    //how often every single network is evaluated to calculate its fitness each round
    unsigned int repetitions;
    //how many generations should be simulated
    unsigned int generation_target;

    //probabilities for different mutation types
    float probability_mutate_link = 0.05f;
    float probability_mutate_node = 0.03f;
    float probability_mutate_weight = 0.8f;
    float probability_mutate_weight_pertube = 0.9f;
    float mutate_weight_pertube_strength = 0.5f;

    //percentage of networks killed off each round
    float elimination_percentage = 0.5f;

    //weights for calculating network distance
    float c1 = 1.0f;
    float c2 = 1.0f;
    float c3 = 0.4f;
    //distance threshhold for when two networks are considered to be of the same species
    float speciation_threshold;

    //activation function for calculation network outputs
    float (*activation_function)(float) = &sigmoid;

    //the path to the folder that holds resulting files of network generations
    string folderpath;

    // ------------ gene providers for networks ------------

    unsigned int node_count;
    vector<bool> used_nodes;

    //returns a connection with the requested weight from node start to node end, registering it with the archives if neccessary
    Connection_Gene request_connection_gene(node_id start, node_id end);
    //returns a connection with the requested id
    Connection_Gene request_connection_gene(connection_id id);

    // ------------ Execution options ------------

    //performs the NEAT algorithm. Each network's fitness is evaluated with the provided function
    void run_neat(int (*evalNetwork)(Network));

    //performs the NEAT algorithm. Each networks's fitness is evaluated by letting them compete with each other network
    //using the provided function and averaging fitness results
    void run_neat(pair<int, int> (*compete_networks)(Network, Network));

    // ------------ Output ------------

    //returns a vector of all managed networks sorted by their last known fitness value in descending order
    vector<Network> get_networks_sorted();

    //prints information about all networks to the standard output
    void print();

    //saves the current generation to the file specified in filepath
    void save() const;

private:
    // ------------ NEAT data  ------------

    //all networks managed by this instance
    vector<Network> networks;
    //the same networks separated into species
    list<Species> species;
    //the archive of all connection among all networks
    unordered_set<Connection_Gene> connection_genes;
    //the current number of simulated generations
    unsigned int generation_number;

    //performs the NEAT-algorithm on the network list.
    //the passed function must set the fitness values of all the networks in the list.
    //should only be called by the public runNeat functions
    void run_neat_helper(const std::function<void()> &evalNetworks);

    //assinges each network in the networks list to a species in the species list. May create new species to accomodate all networks. May remove extince species.
    void assign_networks_to_species();

    static bool sort_by_fitness_desc(const Network &n1, const Network &n2);
};

/*   +------------------------------------------------------------+
 *   |                                                            |
 *   |                          Utility                           |
 *   |                                                            |
 *   +------------------------------------------------------------+
 */


//Helper method that's splits a string into subcomponents. If the string ends with the delimiter, an empty string will NOT be included
vector<string> split(const string &string_to_split, const string &delimiter);


//returns a randomly selected float between the two passed values
float rnd_f(float lo, float hi);


#endif //RAYNEAT_RAYNEAT_H
