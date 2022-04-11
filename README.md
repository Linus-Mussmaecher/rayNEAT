# rayNEAT

A C++ project containing an implementation of the NEAT-algorithm. Uses raylib for visualization.

## Provided Scenarios

- XOR

## How to set up your own scenario

### Setup

This NEAT implementation can be used to learn & perform on your own scenarios.

You can create a NEAT instance with the provided constructor by passing the number of input- and output nodes as well as the desired population.
Afterwards, the following algorithm modifiers can be changed by accessing their fields:

- number of generations
- probability of the different mutation types
  - add node
  - add connection
  - weight changes
    - weight randomization
    - weight shifting
- percentage of population to be cleansed each generation
- speciation threshold
- weight parameters for determining speciation distance
- number of repetitions when determining the fitness of a network
- activation function of the nodes

Changing some of these values can improve the performance of the algorithm on your scenario. Try it out!

While population, input- and output count may be changed by accessing their fields as well, this is not recommended and may yield unexpected results.

### Execution

After the NEAT instance has been created, it can perform the NEAT evolution by calling either of the runNEAT functions.
You may either pass a simple evaluation function that takes in a Network and returns a fitness value to assign to that network or pass a competitive evolution function that takes in two networks and returns a pair of fitness values for the competitors (Values from all network pairings will be averaged to get the final fitness of the network).

Within either type of evaulation function, you can get a networks "decision" based on the current state of your scenario by using the calculate function.
Pass in an array of float values with length equal to the amount of input nodes and representing the current state. The function then returns and array of floats with length equal to the amount of output nodes. The actions of the network in your scenario should then be based on the value of these output nodes.
This process may be repeated multiple times until the scenario has ended. 
(Example: A network trying to guess written letters will only need one calculation per evaluation, while a network playing snake will need to calculate every timestep of the game and then be evaluated after the game has finished).
After the instance of the scenario has finished, rewards your network(s) with fitness based on their performance. Try not to make the rewards to binary (Reward point differences in competition as opposed to win/loss; reward small amounts of points for time survived in snake, etc.) so the algorithm can judge its progress even when not crossing the binary threshold.

When using the default sigmoid activation function, input values should be scaled to be in [-1,1] and output values will always be in [0,1].

### Saving and loading

While running, the algorithm will save every 10th generation to a .rn file in the folder provided by the field "folderpath" (if it has been set).
Evolution may be picked up at a later point by using the provided constructor taking in a file. It will load the node, connection and network data from the file and assign new species.
Then, once again, parameters may be adjusted and runNEAT can be to evolve to the (new and hopefully higher) generation target.
In order to preserve the evolutionary benefits made in the loaded generations, all to radical changes (changing evaluation function or activation function, mostly) are not suggested since the networks so far have evolved towards different goals.
A suggested use case to change evaluation function would be to train 100 generations of your network against a hard-coded AI using single evolution and then letting the "primed" networks compete against each other.
