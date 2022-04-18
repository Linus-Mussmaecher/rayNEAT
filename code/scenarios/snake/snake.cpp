//
// Created by Linus on 14/04/2022.
//

#include "snake.h"

#include <utility>


void testSnake() {

    clock_t time;

    for (int i = 0; i < 1; i++) {
        time = clock();

        Neat_Instance neat = Neat_Instance(5, 3, 150);
        neat.generation_target = 150;
        neat.node_count_exponent = 0.1;
        if (i >= 15) neat.activation_function = &tanhf;
        neat.folderpath = "./snake_" + std::to_string(i);
        neat.run_neat(&test_network_snake);

        std::cout << "\n\n Finished first run in " << float(clock() - time) / CLOCKS_PER_SEC << "s.\n\n";

    }


}


void visualize_snake() {

    Neat_Instance neat = Neat_Instance("./snake_0/NEAT_Generation_50.rn");

    Network n = neat.get_networks_sorted()[0];

    InitWindow(800, 600, "Snake");
    SetTargetFPS(8);

    AI_Snake_Agent asn(n);
    User_Snake_Agent usn;
    Snake_Game(reinterpret_cast<Snake_Agent *>(&asn), 15, 15).run_visual();

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(WHITE);
        n.draw({20,20,760,560});
        EndDrawing();
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

Snake_Game::Snake_Game(Snake_Agent *agent, const int w, const int h) : agent(agent), w(w), h(h), score(0),
                                                                       stagnation_counter(0), food({w / 2, h / 2}),
                                                                       snake() {
    snake.push_front({1, 1});
    snake.push_front({2, 1});
}

bool Snake_Game::step() {
    bool running = true;
    pos next = snake.front() + agent->getNextDirection(*this);
    //check for food
    if (next == food) {
        food = {GetRandomValue(0, w - 1), GetRandomValue(0, h - 1)};
        score++;
        stagnation_counter = 0;
    } else {
        snake.pop_back();
    }
    stagnation_counter++;
    //check for out of bounds
    if (next.x < 0 || next.y < 0 || next.x >= w || next.y >= h) {
        running = false;
    }
    //check for self-collision
    if (std::find(snake.begin(), snake.end(), next) != snake.end()) {
        running = false;
    }

    snake.push_front(next);

    return running && stagnation_counter < 60 + score * 5;
}


bool Snake_Game::is_obstacle(pos to_check) const {
    return to_check.x < 0 || to_check.x >= w || to_check.y < 0 || to_check.y >= h ||
           std::find(snake.begin(), snake.end(), to_check) != snake.end();
}

float Snake_Game::run() {
    while (step());

    float max_distance = sqrtf(float(w * w + h * h));
    pos foo_rel = food - snake.front();
    float foo_distance = sqrtf(float(foo_rel.x * foo_rel.x + foo_rel.y * foo_rel.y));

    //return score + [0,1[ bonus based on distance to next food (to reward more incremental progression)
    return float(score) + (1.f - foo_distance / max_distance);
}

float Snake_Game::run_visual() {
    while (step() && !WindowShouldClose()) {
        BeginDrawing();
        draw({0, 0, float(GetScreenWidth()), float(GetScreenHeight())});
        EndDrawing();
    };

    float max_distance = sqrtf(float(w * w + h * h));
    pos foo_rel = food - snake.front();
    float foo_distance = sqrtf(float(foo_rel.x * foo_rel.x + foo_rel.y * foo_rel.y));

    //return score + [0,1[ bonus based on distance to next food (to reward more incremental progression)
    return float(score) + (1.f - foo_distance / max_distance);
}

void Snake_Game::draw(Rectangle target) {
    DrawRectangle(
            int(target.x), int(target.y),
            int(target.width), int(target.height),
            WHITE
    );

    int squaresize = int(std::min(target.width / float(w), target.height / float(h)));

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            DrawRectangle(
                    int(target.x) + i * squaresize,
                    int(target.y) + j * squaresize,
                    squaresize, squaresize,
                    BLACK
            );
            DrawRectangle(
                    int(target.x) + i * squaresize + 1,
                    int(target.y) + j * squaresize + 1,
                    squaresize - 2, squaresize - 2,
                    BROWN
            );
        }
    }

    for (const pos &p: snake) {
        DrawRectangle(
                int(target.x) + p.x * squaresize,
                int(target.y) + p.y * squaresize,
                squaresize, squaresize,
                p == snake.front() ? DARKGREEN : GREEN
        );
    }

    DrawRectangle(
            int(target.x) + food.x * squaresize,
            int(target.y) + food.y * squaresize,
            squaresize, squaresize,
            RED
    );
    pos dir = snake.front() - *std::next(snake.begin(), 1);
    pos to_food = food - snake.front();


    float max_distance = sqrtf(float(w * w + h * h));
    pos foo_rel = food - snake.front();
    float foo_distance = sqrtf(float(foo_rel.x * foo_rel.x + foo_rel.y * foo_rel.y));

    /*
    std::cout << "Data: " << (is_obstacle(snake.front() + dir) ? 1.f : 0.f) << " | " <<
              (is_obstacle(snake.front() + pos{dir.y, -dir.x}) ? 1.f : 0.f) << " | " <<
              (is_obstacle(snake.front() + pos{-dir.y, dir.x}) ? 1.f : 0.f) << " | " <<
              atan2(
                      float(dir.x * to_food.y - dir.y * to_food.x),
                      float(dir.x * to_food.x + dir.y * to_food.y)
              ) / PI << " | " <<
              (1.f - foo_distance / max_distance) << " | " <<
              1.f << "\n";
   */
}


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

AI_Snake_Agent::AI_Snake_Agent(Network client) : client(std::move(client)) {}

pos AI_Snake_Agent::getNextDirection(const Snake_Game &state) {
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


float test_network_snake(Network n) {
    AI_Snake_Agent asn(std::move(n));
    return Snake_Game(reinterpret_cast<Snake_Agent *>(&asn), 31, 31).run();
}
