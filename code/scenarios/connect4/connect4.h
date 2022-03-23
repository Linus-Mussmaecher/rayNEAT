//
// Created by Linus on 22.03.2022.
//

#ifndef RAYNEAT_CONNECT4_H
#define RAYNEAT_CONNECT4_H

#include "../../NEAT/rayNEAT.h"

class Connect4Agent{
public:
    virtual unsigned int getPlay(array<array<int, 10>, 10> gamestate, int color) = 0;
};

class NetworkConnect4Agent : public Connect4Agent{
public:
    explicit NetworkConnect4Agent(Network n);
    unsigned int getPlay(array<array<int, 10>, 10> gamestate, int color) override;
private:
    Network network;
};

class PlayerConnect4Agent : public Connect4Agent{
public:
    unsigned int getPlay(array<array<int, 10>, 10> gamestate, int color) override;
};

pair<int, int> playConnect4(Connect4Agent* a, Connect4Agent* b);

pair<int,int> competeConnect4(Network n1, Network n2);

void testConnect4();


#endif //RAYNEAT_CONNECT4_H
