//
// Created by Linus on 14/04/2022.
//

#include "snake.h"

#include <utility>


void evolve_snake() {

    clock_t time;

    for (int i = 0; i < 20; i++) {
        time = clock();

        Neat_Instance neat = Neat_Instance(5, 3, 250);
        neat.generation_target = 250;
        neat.node_count_exponent = 0.1;
        neat.repetitions = 100;
        if (i >= 15) neat.activation_function = &tanhf;
        neat.folderpath = "./snake_" + std::to_string(i);
        neat.run_neat(&test_network_snake);

        std::cout << "\n\n Finished first run in " << float(clock() - time) / CLOCKS_PER_SEC << "s.\n\n";
    }


}


void visualize_snake() {
    Neat_Instance neat = Neat_Instance("./snake_0/NEAT_Generation_150.rn");

    Network n = neat.get_networks_sorted()[0];

    InitWindow(800, 600, "Snake");
    SetTargetFPS(8);

    Basic_AI_Snake_Agent asn(n);
    User_Snake_Agent usn;
    float score = Snake_Game(reinterpret_cast<Snake_Agent *>(&asn), 16, 16).run_visual();
    std::cout << "Fitness-Score: " << score << "\n";
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

Snake_Game::Snake_Game(Snake_Agent *agent, const int w, const int h) : agent(agent), w(w), h(h), score(0), fitness(0),
                                                                       food_dist(w / 2 + h / 2 - 3),
                                                                       stagnation_counter(0),
                                                                       food({w / 2, h / 2}),
                                                                       snake() {
    //init first two body parts of snake to ensure pairwise iteration does not throw errors
    snake.push_front({1, 1});
    snake.push_front({2, 1});
}

bool Snake_Game::step() {
    bool running = true;
    pos next = snake.front() + agent->getNextDirection(*this);
    //check for food
    if (next == food) {
        //calculate increase to fitness
        //base for finding the food
        float base_reward = 0.6f; //0.5f - 0.4f * std::max(1.f, float(score) / 20.f);
        //rest is awarded for efficiency. shortest possible path -> full point. Double length or longer -> no bonus
        fitness += base_reward +
                   (1.f - base_reward) * std::max(1.f - float(stagnation_counter - food_dist) / float(food_dist), 0.f);
        //increase score
        score++;
        //re-place food
        food = {GetRandomValue(0, w - 1), GetRandomValue(0, h - 1)};
        food_dist = abs(food.x - next.x) + abs(food.y - next.y);
        //reset stagnation time-out
        stagnation_counter = 0;
    } else {
        snake.pop_back();
    }
    stagnation_counter++;

    if (is_obstacle(next)) {
        running = false;
    }

    snake.push_front(next);

    return running && stagnation_counter < food_dist + w + h;
}

float test_network_snake(Network n) {
    Basic_AI_Snake_Agent asn(std::move(n));
    return Snake_Game(reinterpret_cast<Snake_Agent *>(&asn), 31, 31).run();
}

float Snake_Game::run() {
    while (step());

    float max_distance = sqrtf(float(w * w + h * h));
    pos foo_rel = food - snake.front();
    float foo_distance = sqrtf(float(foo_rel.x * foo_rel.x + foo_rel.y * foo_rel.y));

    //return score + [0,1[ bonus based on distance to next food (to reward more incremental progression)
    return float(fitness) + (1.f - foo_distance / max_distance);
}

float Snake_Game::run_visual() {
    while (step() && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        draw({10, 10, float(GetScreenWidth() - 20), float(GetScreenHeight() - 20)});
        EndDrawing();
    }

    float max_distance = sqrtf(float(w * w + h * h));
    pos foo_rel = food - snake.front();
    float foo_distance = sqrtf(float(foo_rel.x * foo_rel.x + foo_rel.y * foo_rel.y));

    //return score + [0,1[ bonus based on distance to next food (to reward more incremental progression)
    return float(fitness) + (1.f - foo_distance / max_distance);
}


bool Snake_Game::is_obstacle(pos to_check) const {
    //check for out-of-bounds and self-collision
    return to_check.x < 0 || to_check.x >= w || to_check.y < 0 || to_check.y >= h ||
           std::find(snake.begin(), snake.end(), to_check) != snake.end();
}

int Snake_Game::obstacle_ray(pos start, pos dir) const {
    int d = 0;
    while(!is_obstacle(start)){
        start = start + dir;
        d++;
    }
    return d;
}

inline float Snake_Game::diagonal() const{
    return sqrtf(float(w * w + h * h));
}


void Snake_Game::draw(Rectangle target) {

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

    auto p1 = snake.begin();
    auto p2 = std::next(snake.begin());

    while (p2 != snake.end()) {
        //body
        DrawRectangle(
                int(target.x) + std::min(p1->x, p2->x) * squaresize + squaresize / 4,
                int(target.y) + std::min(p1->y, p2->y) * squaresize + squaresize / 4,
                (p1->x == p2->x ? 1 : 3) * squaresize / 2,
                (p1->x == p2->x ? 3 : 1) * squaresize / 2,
                GREEN
        );
        if (p1 == snake.begin()) {
            //eyes
            DrawRectangle(
                    int(target.x) + p1->x * squaresize + squaresize / 2 - squaresize / 16
                    + (p1->y - p2->y) * squaresize / 8,
                    int(target.y) + p1->y * squaresize + squaresize / 2 - squaresize / 16
                    + (p1->x - p2->x) * squaresize / 8,
                    squaresize / 8,
                    squaresize / 8,
                    BLUE
            );
            DrawRectangle(
                    int(target.x) + p1->x * squaresize + squaresize / 2 - squaresize / 16
                    + (p2->y - p1->y) * squaresize / 8,
                    int(target.y) + p1->y * squaresize + squaresize / 2 - squaresize / 16
                    + (p2->x - p1->x) * squaresize / 8,
                    squaresize / 8,
                    squaresize / 8,
                    BLUE
            );
            //tongue
            if (abs(food.x - p1->x) + abs(food.y - p1->y) <= 3) {
                DrawRectangle(
                        int(target.x) + p1->x * squaresize + squaresize / 2 - squaresize / 16
                        + (p1->x - p2->x) * 5 * squaresize / 16,
                        int(target.y) + p1->y * squaresize + squaresize / 2 - squaresize / 16
                        + (p1->y - p2->y) * 5 * squaresize / 16,
                        squaresize / 8,
                        squaresize / 8,
                        RED
                );
            }
        }


        p1++;
        p2++;
    }

    //apple stump
    DrawRectangle(
            int(target.x) + food.x * squaresize + squaresize / 2 - squaresize / 16,
            int(target.y) + food.y * squaresize + 1,
            squaresize / 8, squaresize / 2,
            DARKGREEN
    );
    //apple main
    DrawCircle(
            int(target.x) + food.x * squaresize + squaresize / 2,
            int(target.y) + food.y * squaresize + squaresize / 2,
            float(squaresize) / 3.f,
            RED
    );
}
