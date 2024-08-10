#include <iostream>
#include <random>
#include <ctime>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Simulation settings
const long POPULATION_SIZE = 100000;
const int INITIAL_INFECTED = 25;
const int DAYS = 250000;
const int INITIAL_WIDTH = 800;
const int INITIAL_HEIGHT = 600;
const int DOT_RADIUS = 5;
const int DOUGHNUT_HOVER_RADIUS = 15;
const int UPDATE_INTERVAL = 1; // Interval for placing dots on the graph
const int MARGIN = 50;

// Counters for population status
int susceptible = POPULATION_SIZE - INITIAL_INFECTED, infected = INITIAL_INFECTED, recovered = 0, dead = 0;
int mutations = 0;

struct Person {
    short status; // 0 = susceptible, 1 = infected, 2 = recovered, 3 = dead
    bool masked = false; // true if the person wears a mask
};

void virus_mutation(Person population[], float geneticDifference, std::mt19937& gen, std::uniform_real_distribution<>& dist) {
    mutations += 1;
    std::cout << "Virus mutation. Genetic difference: " << geneticDifference << std::endl;
    for (long i = 0; i < POPULATION_SIZE; i++) {
        if ((dist(gen)) > (1 - geneticDifference)) {
            if (population[i].status == 2) {
                population[i].status = 0;
            }
        }
    }
}

void simulate_day(Person population[], long size, std::mt19937& gen, std::uniform_real_distribution<>& dist) {
    for (long i = 0; i < size; i++) {
        if (population[i].status == 1) { // If infected
            for (int j = 0; j < (population[i].masked ? 5 : 25); j++) { // Try to infect 5 random people if masked, otherwise 25
                long randIndex = gen() % size; // Random index
                if (population[randIndex].status == 0) { // If susceptible
                    if (dist(gen) < (population[randIndex].masked ? 0.06 : 0.19)) { // 6% if masked, 19% otherwise
                        population[randIndex].status = 1;
                    }
                }
            }

            if (dist(gen) < 0.05) { // 5% recovery chance
                population[i].status = 2;
            } else if (dist(gen) < 0.01) { // 1% death chance
                population[i].status = 3;
            }
        }
    }

    if (dist(gen) < 0.005) { // 0.5% chance to mutate the virus
        virus_mutation(population, dist(gen), gen, dist);
    }
}

void drawGraph(SDL_Renderer* renderer, const std::vector<int>& infectedHistory, const std::vector<int>& deadHistory, int daysElapsed, int width, int height) {
    int graphWidth = width - 2 * MARGIN;
    int graphHeight = height / 2 - 2 * MARGIN;
    int plotHeight = height / 2 - 2 * MARGIN;

    // Draw infected graph
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for infected
    for (int i = 1; i < daysElapsed; i++) {
        int x1 = MARGIN + (i - 1) * (graphWidth) / daysElapsed;
        int y1 = height / 2 - graphHeight + graphHeight - (infectedHistory[i - 1] * (graphHeight - 20) / POPULATION_SIZE);
        int x2 = MARGIN + i * (graphWidth) / daysElapsed;
        int y2 = height / 2 - graphHeight + graphHeight - (infectedHistory[i] * (graphHeight - 20) / POPULATION_SIZE);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }

    // Draw dead graph below infected graph
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for dead
    for (int i = 1; i < daysElapsed; i++) {
        int x1 = MARGIN + (i - 1) * (graphWidth) / daysElapsed;
        int y1 = height - (height / 2) + MARGIN - (deadHistory[i - 1] * (plotHeight - 20) / POPULATION_SIZE);
        int x2 = MARGIN + i * (graphWidth) / daysElapsed;
        int y2 = height - (height / 2) + MARGIN - (deadHistory[i] * (plotHeight - 20) / POPULATION_SIZE);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

void drawDoughnut(SDL_Renderer* renderer, int x, int y, int radius, SDL_Color color, bool isHovered) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    int outerRadius = isHovered ? DOUGHNUT_HOVER_RADIUS : radius;
    int innerRadius = radius / 2;

    for (int w = -outerRadius; w < outerRadius; w++) {
        for (int h = -outerRadius; h < outerRadius; h++) {
            if (w * w + h * h <= outerRadius * outerRadius && w * w + h * h >= innerRadius * innerRadius) {
                SDL_RenderDrawPoint(renderer, x + w, y + h);
            }
        }
    }
}

void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y) {
    SDL_Color color = {255, 255, 255, 255}; // White color
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (textSurface == nullptr) {
        std::cerr << "Failed to create text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        std::cerr << "Failed to create text texture! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};

    SDL_FreeSurface(textSurface);

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
}

int main() {
    std::clog << "Warning: GUI is deprecated. Please use the CLI." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Pandemic Simulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, INITIAL_WIDTH, INITIAL_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("arial.ttf", 16);
    if (font == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Person* population = new Person[POPULATION_SIZE];
    for (long i = 0; i < POPULATION_SIZE; i++) {
        population[i].status = 0;
    }

    for (int i = 0; i < INITIAL_INFECTED; i++) {
        population[i].status = 1;
    }

    std::vector<int> infectedHistory;
    std::vector<int> deadHistory;
    bool pandemicOver = false;
    int day = 1;
    bool end = false;
    clock_t timeStart = clock();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0.0, 1.0);

    int width = INITIAL_WIDTH;
    int height = INITIAL_HEIGHT;

    while (!end) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                end = true;
                break;
            } else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                width = e.window.data1;
                height = e.window.data2;
            }
        }

        if (pandemicOver && !infectedHistory.empty()) {
            day = DAYS;
        }

        if (!pandemicOver) {
            simulate_day(population, POPULATION_SIZE, gen, dist);

            susceptible = 0;
            infected = 0;
            recovered = 0;
            dead = 0;

            for (long i = 0; i < POPULATION_SIZE; i++) {
                if (population[i].status == 0) susceptible++;
                else if (population[i].status == 1) infected++;
                else if (population[i].status == 2) recovered++;
                else if (population[i].status == 3) dead++;
            }

            if (infected == 0 && !pandemicOver) {
                pandemicOver = true;
            }

            if (day % UPDATE_INTERVAL == 0 && day <= DAYS) {
                infectedHistory.push_back(infected);
                deadHistory.push_back(dead);
            }

            day++;
        }

        // Render the updated graph
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawGraph(renderer, infectedHistory, deadHistory, infectedHistory.size(), width, height);

        SDL_Color hoverColor = {255, 0, 0, 255};
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        for (int i = 0; i < infectedHistory.size(); i += UPDATE_INTERVAL) {
            int x = MARGIN + i * (width - 2 * MARGIN) / infectedHistory.size();
            int y = height / 2 - (infectedHistory[i] * (height / 2 - 2 * MARGIN - 20) / POPULATION_SIZE);

            bool isHovered = mouseX >= x - DOUGHNUT_HOVER_RADIUS && mouseX <= x + DOUGHNUT_HOVER_RADIUS &&
                             mouseY >= y - DOUGHNUT_HOVER_RADIUS && mouseY <= y + DOUGHNUT_HOVER_RADIUS;

            drawDoughnut(renderer, x, y, DOT_RADIUS, hoverColor, isHovered);
            if (isHovered) {
                drawText(renderer, font, std::to_string(infectedHistory[i]), x + 5, y - 20);
            }
        }

        SDL_RenderPresent(renderer);

        if (pandemicOver) {
            std::string finalMessage = "Pandemic simulation finished. Press X to exit.";
            drawText(renderer, font, finalMessage, 10, height - 40);
            SDL_RenderPresent(renderer);
        }
    }

    std::cout << "Pandemic simulation finished." << std::endl;

    clock_t timeEnd = clock();
    double duration = double(timeEnd - timeStart) / CLOCKS_PER_SEC;
    std::cout << "Time taken to simulate: " << duration << std::endl;

    delete[] population;

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
