//
// Created by Linus on 22.03.2022.
//

#include "connect4.h"

#include <utility>

NetworkConnect4Agent::NetworkConnect4Agent(Network n) : network(std::move(n)) {

}

unsigned int NetworkConnect4Agent::getPlay(array<array<int, 10>, 10> gamestate, int color) {
    vector<float> input;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            input.push_back(float(gamestate[i][j] * color));
        }
    }
    input.push_back(1.f);
    network.setInputs(input);
    network.calculateOutputs();
    return std::clamp(int(10 * network.getOutputs()[0]), 0, 9);
}

unsigned int PlayerConnect4Agent::getPlay(array<array<int, 10>, 10> gamestate, int color) {
    std::cout << "Board (you are X):\n";
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            std::cout << (gamestate[i][j] == 0 ? " " : (gamestate[i][j] * color == 1 ? "X" : "O"));
        }
        std::cout << "\n";
    }
    std::cout << "0123456789\n";
    int res;
    std::cin >> res;
    return std::clamp(res, 0, 9);
}

void testConnect4(){
    NeatInstance neatInstance = NeatInstance();
    neatInstance.input_count = 101;
    neatInstance.output_count = 1;
    neatInstance.generation_target = 250;
    neatInstance.repetitions = 1;
    neatInstance.population = 100;
    neatInstance.folderpath = "./C4";
    neatInstance.speciation_threshold = 1.0f;
    neatInstance.runNeat(&competeConnect4);
}


pair<int,int> competeConnect4(Network n1, Network n2){
    NetworkConnect4Agent na1(std::move(n1));
    NetworkConnect4Agent na2(std::move(n2));
    auto [s1, s2] = playConnect4(&na1, &na2);
    return {(2 + s1)*(2 + s1), (2+s2)*(2+s2)};
}


pair<int, int> playConnect4(Connect4Agent *a, Connect4Agent *b) {
    array<array<int, 10>, 10> gamestate{};

    //randomize starting player
    int random = GetRandomValue(0, 1);
    Connect4Agent *p1 = random == 0 ? a : b;
    Connect4Agent *p2 = random == 0 ? b : a;

    int round = 0;
    int winner = 0;
    while (winner == 0 && round++ < 50) {
        //get plays from both players
        auto play1 = p1->getPlay(gamestate, 1);
        for (int i = 9; i >= 0; i--) {
            if (gamestate[i][play1] == 0) {
                gamestate[i][play1] = 1;
                break;
            }
        }
        auto play2 = p2->getPlay(gamestate, -1);
        for (int i = 9; i >= 0; i--) {
            if (gamestate[i][play2] == 0) {
                gamestate[i][play2] = -1;
                break;
            }
        }
        //check for a winner
        //horizontal
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 7; j++) {
                if (gamestate[i][j] != 0 &&
                    gamestate[i][j] == gamestate[i][j + 1] &&
                    gamestate[i][j + 1] == gamestate[i][j + 2] &&
                    gamestate[i][j + 2] == gamestate[i][j + 3]) {
                    winner = gamestate[i][j];
                }
            }
        }
        //vertical
        for (int j = 0; j < 10; j++) {
            for (int i = 0; i < 7; i++) {
                if (gamestate[i][j] != 0 &&
                    gamestate[i][j] == gamestate[i + 1][j] &&
                    gamestate[i + 1][j] == gamestate[i + 2][j] &&
                    gamestate[i + 2][j] == gamestate[i + 3][j]) {
                    winner = gamestate[i][j];
                }
            }
        }
        //diagonal 1
        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 7; j++) {
                if (gamestate[i][j] != 0 &&
                    gamestate[i][j] == gamestate[i + 1][j + 1] &&
                    gamestate[i + 1][j + 1] == gamestate[i + 2][j + 2] &&
                    gamestate[i + 2][j + 2] == gamestate[i + 3][j + 3]) {
                    winner = gamestate[i][j];
                }
            }
        }
        //diagonal 2
        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 7; j++) {
                if (gamestate[i][j] != 0 &&
                    gamestate[i + 3][j] == gamestate[i + 2][j + 1] &&
                    gamestate[i + 2][j + 1] == gamestate[i + 1][j + 2] &&
                    gamestate[i + 1][j + 2] == gamestate[i][j + 3]) {
                    winner = gamestate[i + 3][j];
                }
            }
        }
    }

    if (round == 50) {
        return {0, 0};
    }
    return {random == 0 ? winner : -winner, random == 0 ? -winner : winner};
}
