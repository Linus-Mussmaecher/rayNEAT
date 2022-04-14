//
// Created by Linus on 14/04/2022.
//

#include "snake.h"

#include <utility>

pos operator+(pos a, pos b) {
    return {a.x + b.x, a.y + b.y};
}

pos operator-(pos a, pos b) {
    return {a.x - b.x, a.y - b.y};
}


bool operator==(pos a, pos b) {
    return a.x == b.x && a.y == b.y;
}


int runSnake(Snake_Agent *agent, int w, int h) {
    //Init
    Snake_State state = {w, h, 0, {GetRandomValue(0, w - 1), GetRandomValue(0, h - 1)}, {}};
    bool fail = false;
    int stagnation_counter = 0;

    while (!fail && stagnation_counter <= 60 + state.score * 5) {
        pos next = state.snake.front() + agent->getNextDirection(state);
        //check for food
        if (next == state.food) {
            state.food = {GetRandomValue(0, state.w - 1), GetRandomValue(0, state.h - 1)};
            stagnation_counter = 0;
        } else {
            state.snake.pop_back();
        }
        stagnation_counter++;
        //check for out of bounds
        if (next.x < 0 || next.y < 0 || next.x >= w || next.y >= h) {
            fail = true;
        }
        //check for self-collision
        if (std::find(state.snake.begin(), state.snake.end(), next) != state.snake.end()) {
            fail = true;
        }
    }

    return state.score;
}


pos User_Snake_Agent::getNextDirection(const Snake_State &state) {
    if (IsKeyPressed(KEY_UP)) {
        return {0, -1};
    }
    if (IsKeyPressed(KEY_DOWN)) {
        return {0, 1};
    }
    if (IsKeyPressed(KEY_LEFT)) {
        return {-1, 0};
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        return {1, 0};
    }
    //no key pressed -> move as usual
    return state.snake.front() - *std::next(state.snake.begin(), 1);
}

AI_Snake_Agent::AI_Snake_Agent(Network client) : client(std::move(client)) {}

pos AI_Snake_Agent::getNextDirection(const Snake_State &state) {
    vector<float> res = client.calculate(
            {
                    //TODO: all parameters
                    float(getDistance(state.snake.front(), {0, 1},
                                      [&state](const pos &p) { return p == state.food; }, 31)),
                    float(state.score),
                    1.f
            }
    );

    int max_index = int(std::distance(res.begin(), std::max_element(res.begin(), res.end())));

    if (max_index == 0) {
        return {0, -1};
    }
    if (max_index == 1) {
        return {0, 1};
    }
    if (max_index == 2) {
        return {-1, 0};
    }
    if (max_index == 3) {
        return {1, 0};
    }
    //this should not happen, but move as usual
    return state.snake.front() - *std::next(state.snake.begin(), 1);
}

int getDistance(pos start, pos dir, const std::function<bool(pos)> &check, int max) {
    int res = 0;
    for (pos c = start; res < max; c = c + dir) {
        if (check(c)) {
            return res;
        }
        res++;
    }

    return res;
}
