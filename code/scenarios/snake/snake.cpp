//
// Created by Linus on 14/04/2022.
//

#include "snake.h"

#include <utility>


void testSnake(){

    clock_t time;

    for(int i = 0; i < 20; i++){
        time = clock();

        Neat_Instance neat = Neat_Instance(6, 4, 250);
        neat.generation_target = 250;
        neat.node_count_exponent = 0.1;
        if(i >= 15) neat.activation_function = &tanhf;
        neat.folderpath = "./snake_" + std::to_string(i);
        neat.run_neat(&test_network_snake);

        std::cout << "\n\n Finished first run in " << float(clock() - time)/CLOCKS_PER_SEC << "s.\n\n";

    }


}


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
    Snake_State state = {w, h, 0, {5,5}, { {1,1}, {2,1} }};
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

        state.snake.push_front(next);
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
    pos dir = state.snake.front() - *std::next(state.snake.begin(), 1);
    pos to_food = state.food - state.snake.front();

    vector<float> res = client.calculate(
            {
                    is_obstacle(state, state.snake.front() + dir) ? 1.f : 0.f, //obstacle in front
                    is_obstacle(state, state.snake.front() + pos{dir.y, -dir.x}) ? 1.f : 0.f, //obstacle to the left
                    is_obstacle(state, state.snake.front() + pos{-dir.y, dir.x}) ? 1.f : 0.f, //obstacle to the right
                    atan2( float(dir.x*to_food.y - dir.y*to_food.x), float(dir.x*to_food.x + dir.y*to_food.y )) / PI, //angle to food
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
    return dir;
}


bool is_obstacle(Snake_State state, pos to_check) {
    return to_check.x < 0 || to_check.x >= state.w || to_check.y < 0 || to_check.y >= state.h || std::find(state.snake.begin(), state.snake.end(), to_check) != state.snake.end();
}


int test_network_snake(Network n){
    AI_Snake_Agent asn(std::move(n));
    return runSnake(reinterpret_cast<Snake_Agent *>(&asn), 31, 31);
}
