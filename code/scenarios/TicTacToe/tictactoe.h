//
// Created by Linus on 22.03.2022.
//

#ifndef RAYNEAT_TICTACTOE_H
#define RAYNEAT_TICTACTOE_H

#include "../../NEAT/rayNEAT.h"

class TTTAgent{
public:
    virtual pair<int,int> getPlay(array<array<int, 3>, 3> gamestate, int color) = 0;
};

class NetworkTTTAgent : public TTTAgent{
public:
    explicit NetworkTTTAgent(Network n);
    pair<int,int> getPlay(array<array<int, 3>, 3> gamestate, int color) override;
private:
    Network network;
};

class PlayerTTTAgent : public TTTAgent{
public:
    pair<int,int> getPlay(array<array<int, 3>, 3> gamestate, int color) override;
};

pair<int, int> playTTT(TTTAgent* a, TTTAgent* b, bool output = false);

pair<int,int> competeTTT(Network n1, Network n2);

void testTTT();

#endif //RAYNEAT_TICTACTOE_H
