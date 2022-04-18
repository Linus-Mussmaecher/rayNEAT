//
// Created by Linus on 14/04/2022.
//

#ifndef RAYNEAT_SNAKE_H
#define RAYNEAT_SNAKE_H

#include "../../NEAT/rayNEAT.h"

class Snake_Game;

struct pos{
    int x;
    int y;
};

pos operator+(pos a, pos b);

pos operator-(pos a, pos b);

bool operator==(pos a, pos b);

class Snake_Agent{
public:
    virtual pos getNextDirection(const Snake_Game &state) = 0;
};

class User_Snake_Agent{
public:
    virtual pos getNextDirection(const Snake_Game &state);
};

class AI_Snake_Agent{
public:
    explicit AI_Snake_Agent(Network client);

    virtual pos getNextDirection(const Snake_Game &state);
private:
    Network client;
};

class Snake_Game {
private:
public:
    Snake_Game(Snake_Agent* agent, int w, int h);

    bool step();

    void draw(Rectangle target);

    float run();
    float run_visual();

    [[nodiscard]] bool is_obstacle(pos to_check) const;

    list<pos> snake;
    pos food;
private:
    const int w;
    const int h;

    int score;
    int stagnation_counter;

    Snake_Agent* agent;
};

void testSnake();

void visualize_snake();

float test_network_snake(Network n);


#endif //RAYNEAT_SNAKE_H
