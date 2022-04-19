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

class Basic_AI_Snake_Agent{
public:
    explicit Basic_AI_Snake_Agent(Network client);

    virtual pos getNextDirection(const Snake_Game &state);

    static float test(const Network& n);
private:
    Network client;
};

class Ray_AI_Snake_Agent{
public:
    explicit Ray_AI_Snake_Agent(Network client);

    virtual pos getNextDirection(const Snake_Game &state);

    static float test(const Network& n);
private:
    Network client;
};


class Snake_Game {
private:
public:
    Snake_Game(Snake_Agent* agent, int w, int h);

    //runs a game of snake, returning the fitness.
    float run();
    //runs a game equivalent to the run() function while drawing every step to the raylib buffer 1 step = 1 frame
    float run_visual();
    //returns wether or not the selected position is a failstate (snake body or out-of-bounds)
    [[nodiscard]] bool is_obstacle(pos to_check) const;
    //returns the distance to the next square that is blocked in dir-steps. Does not check start itself
    [[nodiscard]] int obstacle_ray(pos start, pos dir) const;
    //returns the diagonal of the playing field
    [[nodiscard]] inline float diagonal() const;
    //a list of coordinates that correspond to the body parts of the snake
    list<pos> snake;
    //the position of the current food-target
    pos food;
private:
    //performs a game step
    bool step();
    //draws the current gamestate to the raylib buffer BeginDrawing() must already be entered
    void draw(Rectangle target);

    //the width of the playing field
    const int w;
    //the height of the playing field
    const int h;
    //the amount of food already collected
    int score;
    //the amount of moves performed since the last food consumption
    int stagnation_counter;
    //the distance from the snake-head to the food at the time of food placement
    int food_dist;
    //the "fitness" of the current agent, taking into account score and efficiency
    float fitness;
    //the snake agent from wich the moves are requested
    Snake_Agent* agent;
};

void evolve_snake();

void visualize_snake();


#endif //RAYNEAT_SNAKE_H
