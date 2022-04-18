//
// Created by Linus on 14/04/2022.
//

#ifndef RAYNEAT_SNAKE_H
#define RAYNEAT_SNAKE_H

#include "../../NEAT/rayNEAT.h"

struct pos{
    int x;
    int y;
};

pos operator+(pos a, pos b);

pos operator-(pos a, pos b);

bool operator==(pos a, pos b);

struct Snake_State {
    const int w;
    const int h;

    int score;

    pos food;
    list<pos> snake;
};

class Snake_Agent{
public:
    virtual pos getNextDirection(const Snake_State &state) = 0;
};

class User_Snake_Agent{
public:
    virtual pos getNextDirection(const Snake_State &state);
};

class AI_Snake_Agent{
public:
    explicit AI_Snake_Agent(Network client);

    virtual pos getNextDirection(const Snake_State &state);
private:
    Network client;
};

void testSnake();

float runSnake(Snake_Agent* agent, int w = 31, int h = 31);

float test_network_snake(Network n);

//returns if the passed x/y coordinates are out of bounds of part of the snake
bool is_obstacle(Snake_State state, pos to_check);


#endif //RAYNEAT_SNAKE_H
