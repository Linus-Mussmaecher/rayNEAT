//
// Created by Linus on 19/04/2022.
//

#include "snake.h"

pos User_Snake_Agent::getNextDirection(const Snake_Game &state) {
    int key = GetKeyPressed();
    if (key == KEY_UP) {
        return {0, -1};
    }
    if (key == KEY_DOWN) {
        return {0, 1};
    }
    if (key == KEY_LEFT) {
        return {-1, 0};
    }
    if (key == KEY_RIGHT) {
        return {1, 0};
    }
    //no key pressed -> move as usual
    return state.snake.front() - *std::next(state.snake.begin(), 1);
}

Basic_AI_Snake_Agent::Basic_AI_Snake_Agent(Network client) : client(std::move(client)) {}

pos Basic_AI_Snake_Agent::getNextDirection(const Snake_Game &state) {
    pos dir = state.snake.front() - *std::next(state.snake.begin(), 1);
    pos to_food = state.food - state.snake.front();

    vector<float> res = client.calculate(
            {
                    state.is_obstacle(state.snake.front() + dir) ? 1.f : 0.f, //obstacle in front
                    state.is_obstacle(state.snake.front() + pos{dir.y, -dir.x}) ? 1.f : 0.f, //obstacle to the left
                    state.is_obstacle(state.snake.front() + pos{-dir.y, dir.x}) ? 1.f : 0.f, //obstacle to the right
                    atan2(
                            float(dir.x * to_food.y - dir.y * to_food.x),
                            float(dir.x * to_food.x + dir.y * to_food.y)
                    ) / PI, //angle to food
                    1.f
            }
    );

    int max_index = int(std::distance(res.begin(), std::max_element(res.begin(), res.end())));

    if (max_index == 0) {
        return {dir.y, -dir.x};
    }
    if (max_index == 1) {
        return dir;
    }
    if (max_index == 2) {
        return {-dir.y, dir.x};
    }
    //this should not happen, but move as usual
    return dir;
}

Ray_AI_Snake_Agent::Ray_AI_Snake_Agent(Network client) : client(std::move(client)) {}

pos Ray_AI_Snake_Agent::getNextDirection(const Snake_Game &state) {
    pos dir = state.snake.front() - *std::next(state.snake.begin(), 1);
    pos to_food = state.food - state.snake.front();

    vector<float> res = client.calculate(
            {
                    float(state.obstacle_ray(state.snake.front(), dir)) / state.diagonal(), //obstacle front
                    float(state.obstacle_ray(state.snake.front(),  pos{dir.y, -dir.x})) / state.diagonal(), //obstacle left
                    float(state.obstacle_ray(state.snake.front(),  pos{-dir.y, dir.x})) / state.diagonal(), //obstacle right
                    atan2(
                            float(dir.x * to_food.y - dir.y * to_food.x),
                            float(dir.x * to_food.x + dir.y * to_food.y)
                    ) / PI, //angle to food
                    sqrtf(float(to_food.x * to_food.x + to_food.y * to_food.y)) / state.diagonal(), //normalized distance to food
                    1.f
            }
    );

    int max_index = int(std::distance(res.begin(), std::max_element(res.begin(), res.end())));

    if (max_index == 0) {
        return {dir.y, -dir.x};
    }
    if (max_index == 1) {
        return dir;
    }
    if (max_index == 2) {
        return {-dir.y, dir.x};
    }
    //this should not happen, but move as usual
    return dir;
}