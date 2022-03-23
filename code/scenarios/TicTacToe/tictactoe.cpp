//
// Created by Linus on 22.03.2022.
//

#include "tictactoe.h"

#include <utility>

NetworkTTTAgent::NetworkTTTAgent(Network n) : network(std::move(n)) {

}

pair<int, int> NetworkTTTAgent::getPlay(array<array<int, 3>, 3> gamestate, int color) {
    vector<float> input;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            input.push_back(float(gamestate[i][j] * color));
        }
    }
    input.push_back(1.f);
    network.setInputs(input);
    network.calculateOutputs();
    return {
            std::clamp(int(3 * network.getOutputs()[0]), 0, 2),
            std::clamp(int(3 * network.getOutputs()[1]), 0, 2),
    };
}

pair<int, int> PlayerTTTAgent::getPlay(array<array<int, 3>, 3> gamestate, int color) {
    std::cout << "Board (you are X):\n";
    for (int i = 0; i < 3; i++) {
        std::cout << "+-+-+-+\n|";
        for (int j = 0; j < 3; j++) {
            std::cout << (gamestate[i][j] == 0 ? " " : (gamestate[i][j] * color == 1 ? "X" : "O")) << "|";
        }
        std::cout << "\n";
    }
    std::cout << "+-+-+-+\n";
    int z;
    std::cin >> z;
    int s;
    std::cin >> s;
    return {
            std::clamp(z, 0, 2),
            std::clamp(s, 0, 2),
    };
}

void testTTT() {
    NeatInstance neatInstance = NeatInstance();
    neatInstance.input_count = 10;
    neatInstance.output_count = 2;
    neatInstance.generation_target = 100;
    neatInstance.repetitions = 3;
    neatInstance.population = 100;
    neatInstance.folderpath = "./TTT";
    neatInstance.speciation_threshold = 1.0f;
    neatInstance.runNeat(&competeTTT);
}


pair<int, int> competeTTT(Network n1, Network n2) {
    NetworkTTTAgent na1(std::move(n1));
    NetworkTTTAgent na2(std::move(n2));
    auto[s1, s2] = playTTT(&na1, &na2);
    return {(2 + s1) * (2 + s1), (2 + s2) * (2 + s2)};
}


pair<int, int> playTTT(TTTAgent *a, TTTAgent *b, bool output) {
    array<array<int, 3>, 3> gamestate{};

    int round = 0;
    int winner = 0;
    int valid = 0;

    while (winner == 0 && round++ < 50 && valid < 9) {
        //get plays from both players
        pair<int,int> play;
        if (round % 2 == 0) {
            play = a->getPlay(gamestate, 1);
        } else {
            play = b->getPlay(gamestate, -1);
        }
        if (gamestate[play.first][play.second] == 0) {
            gamestate[play.first][play.second] = round % 2 == 0 ? 1 : -1;
            valid++;
        }
        if(output){
            std::cout << "Playing " << (round %2 == 0 ? "X" : "O") << " on " << play.first << "-" << play.second << ".\n";
            for (int i = 0; i < 3; i++) {
                std::cout << "+-+-+-+\n|";
                for (int j = 0; j < 3; j++) {
                    std::cout << (gamestate[i][j] == 0 ? " " : (gamestate[i][j] == 1 ? "X" : "O")) << "|";
                }
                std::cout << "\n";
            }
            std::cout << "+-+-+-+\n";
        }
        //check for a winner
        for (int i = 0; i < 3; i++) {
            //horizontal
            if (gamestate[i][0] != 0 && gamestate[i][0] == gamestate[i][1] && gamestate[i][1] == gamestate[i][2]) {
                winner = gamestate[i][0];
            }
            //vertical
            if (gamestate[i][0] != 0 && gamestate[0][i] == gamestate[1][i] && gamestate[1][i] == gamestate[2][i]) {
                winner = gamestate[0][i];
            }
        }
        //diagonals
        if (gamestate[0][0] != 0 && gamestate[0][0] == gamestate[1][1] && gamestate[1][1] == gamestate[2][2]) {
            winner = gamestate[0][0];
        }
        if (gamestate[0][2] != 0 && gamestate[0][2] == gamestate[1][1] && gamestate[1][1] == gamestate[2][0]) {
            winner = gamestate[0][2];
        }
    }
    if (winner == 0){
      if(valid == 9){
          //if the game timed out because it is a legal draw -> return draw
          return {0,0};
      }else{
          //if both made illegal moves until 50 rounds were done, penalize both
          return {-1,-1};
      }

    }else {
        //if the game ended legally -> return winner/loser
        return {winner, -winner};
    }
}