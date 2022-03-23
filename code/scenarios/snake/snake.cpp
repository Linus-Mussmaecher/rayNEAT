//
// Created by Linus on 22.03.2022.
//

#include "snake.h"

#include <utility>


void testSnake() {
    NeatInstance neatInstance = NeatInstance();
    neatInstance.input_count = 4;
    neatInstance.output_count = 4;
    neatInstance.generation_target = 200;
    neatInstance.population = 100;
    neatInstance.speciation_threshold = 1.5f;
    neatInstance.folderpath = "./snake";
    neatInstance.runNeat(&runSnake);
}


int runSnake(Network n) {
    return runSnakeVisual(std::move(n), false);
}

float tailDistanceInDirection(list<pair<int, int>> snake, pair<int, int> direction) {
    pair<int, int> closest;
    for (int i = 1; i < 12; i++) {
        closest = {snake.begin()->first + i * direction.first, snake.begin()->second + i * direction.second};
        if (!std::none_of(snake.begin(), snake.end(), [closest](auto p) { return p == closest; })) {
            break;
        }
    }
    return sqrtf(float((snake.begin()->first - closest.first) * (snake.begin()->first - closest.first)) +
                 float((snake.begin()->second - closest.second) * (snake.begin()->second - closest.second)));
}

int runSnakeVisual(Network n, bool visual) {
    list<pair<int, int>> snake;
    snake.emplace_back(2, 2);
    pair<int, int> direction = {1, 0};
    int score = 0;
    int steps = 0;
    pair<int, int> fruit = {3,3};
    bool collision = false;

    if (visual) {
        //init raylib stuff
        InitWindow(800, 600, "Snake");
        SetTargetFPS(4);
    }

    while (!collision && steps < (score + 1) * 50) {
        //get the current head position
        pair<int, int> head = *snake.begin();
        //get direction
        //set inputs
        vector<float> inputs = {
                float(head.first), //wall N
                float(12 - head.first), //wall S
                float(head.second), //wall W
                float(12 - head.second), //wall E
                tailDistanceInDirection(snake, {0, 1}),
                tailDistanceInDirection(snake, {0, -1}),
                tailDistanceInDirection(snake, {1, 0}),
                tailDistanceInDirection(snake, {-1, 0}),
                tailDistanceInDirection(snake, {1, 1}),
                tailDistanceInDirection(snake, {-1, 1}),
                tailDistanceInDirection(snake, {0, -1}),
                tailDistanceInDirection(snake, {-1, -1}),
                float(direction.first),
                float(direction.second),
                float(score),
                float(head.first - fruit.first), //fruit E/W
                float(head.second - fruit.second), //fruit N/S
                float(sqrtf(float((head.first - fruit.first) * (head.first - fruit.first)) +
                            float((head.second - fruit.second) * (head.second - fruit.second)))) //fruit distance
        };
        n.setInputs(inputs);
        n.calculateOutputs();
        //get index of biggest output
        float max_value = 0.f;
        int max_index = 0;
        for (int i = 0; i < 4; i++) {
            if (max_value < n.getOutputs()[i]) {
                max_index = i;
                max_value = n.getOutputs()[i];
            }
        }
        //set direction vector
        if (max_index == 0) direction = {1, 0};
        if (max_index == 1) direction = {-1, 0};
        if (max_index == 2) direction = {0, 1};
        if (max_index == 3) direction = {0, -1};

        //get the next head position
        pair<int, int> next = {head.first + direction.first, head.second + direction.second};

        //check for collisions
        //out of bounds
        if (next.first < 0 || next.first > 12 || next.second < 0 || next.second > 12) {
            collision = true;
        }
        //self-collision
        if (!std::none_of(snake.begin(), snake.end(), [next](auto p) { return p == next; })) {
            collision = true;
        }

        //no collisions -> move the snake
        snake.push_front(next);
        if (std::none_of(snake.begin(), snake.end(), [fruit](auto p) { return p == fruit; })) {
            //if no fruit collision, shorten the tail
            snake.pop_back();
        } else {
            //if fruit collision, increase score and reposition fruit
            score++;
            fruit = {GetRandomValue(0, 12), GetRandomValue(0, 12)};
        }
        steps++;

        if (visual) {
            BeginDrawing();
            ClearBackground(BLACK);
            //borders
            for (int i = -1; i <= 13; i++) {
                DrawRectangle(0, i * 30 + 30, 30, 30, WHITE);
                DrawRectangle(30 + 13 * 30, i * 30 + 30, 30, 30, WHITE);
            }
            for (int i = -1; i <= 13; i++) {
                DrawRectangle(i * 30 + 30, 0, 30, 30, WHITE);
                DrawRectangle(i * 30 + 30, 30 + 13 * 30, 30, 30, WHITE);
            }
            //snake
            for(pair<int,int> &p : snake){
                DrawRectangle(p.first * 30 + 30, p.second * 30 + 30, 30, 30, GREEN);
            }
            DrawRectangle(fruit.first * 30 + 30, fruit.second * 30 + 30, 30, 30, RED);

            EndDrawing();
        }
    }


    if (visual) {
        //init raylib stuff
        CloseWindow();
    }

    return score * 100 + steps;
}