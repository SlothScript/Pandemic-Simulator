#include <iostream>  // I/O
#include <random>    // RNG
#include <ctime>     // Time

// Simulation settings
const long POPULATION_SIZE = 50000;
const int INITIAL_INFECTED = 5;
const int DAYS = 25000;
const float INFECTION_CHANCE = 0.1f;
const float FATALITY = 0.01f;
const float RECOVERY_CHANCE = 0.05f;
const int maxInfection = 10;

// Counters for population status
int susceptible = POPULATION_SIZE - INITIAL_INFECTED, infected = INITIAL_INFECTED, recovered = 0, dead = 0;
int mutations = 0;

double generate_intelligence_score() {
    double mean = 100.0;
    double stddev = 35.0;

    // Initialize random number generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    // Generate two uniform random numbers
    double u1 = dis(gen);
    double u2 = dis(gen);

    // Apply Box-Muller transform
    double z0 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
    
    // Scale and shift the result to match the desired mean and standard deviation
    return z0 * stddev + mean;
}

struct Person {
    short status; // 0 = susceptible, 1 = infected, 2 = recovered, 3 = dead, ... can go up to 32,000
    bool masked = false; // true if the person wears a mask.
    bool quarantined = false; // When quarantined it limits max infection.
    int intelligence = generate_intelligence_score();
};

void virus_mutation(Person population[], float geneticDifference, std::mt19937& gen, std::uniform_real_distribution<>& dist) {
    mutations += 1;
    std::cout << "Virus mutation. Genetic difference: " << geneticDifference << std::endl;
    for (long i  = 0; i < POPULATION_SIZE; i++) {
        if ((dist(gen)) > (1 - geneticDifference)) {
            if (population[i].status == 2) {
                population[i].status = 0;
            }
        }
    }
}

// Function to simulate one day
void simulate_day(Person population[], long size, std::mt19937& gen, std::uniform_real_distribution<>& dist) {
    for (long i = 0; i < size; i++) {
        if (population[i].intelligence >= 105 && population[i].status == 1) {
            // Infected and going to quarantine.
            if (dist(gen) >= 0.65) {
                population[i].quarantined = true;
            }
        }

        if (population[i].status == 1) { // If infected
            // Spread infection to a few randomly selected individuals
            for (int j = 0; j < (population[i].quarantined ? 1 : (population[i].masked ? maxInfection / 2 : maxInfection * 2)); j++) { // Try to infect 10 random people
                long randIndex = gen() % size; // Random index
                if (population[randIndex].status == 0) { // If susceptible
                    if (dist(gen) < (population[randIndex].masked ? INFECTION_CHANCE-0.12f : INFECTION_CHANCE)) { // 10% infection chance
                        population[randIndex].status = 1;
                    }
                }
            }

            // Recovery or death
            if (dist(gen) < RECOVERY_CHANCE) { // 5% recovery chance
                population[i].status = 2;
            } else if (dist(gen) < FATALITY) { // 1% death chance
                population[i].status = 3;
            }
        }
    }

    if (dist(gen) < 0.005) { // 0.5% chance to mutate the virus
        virus_mutation(population, dist(gen), gen, dist);
    }
}

// Function to print statistics
void print_statistics(int day, int susceptible, int infected, int recovered, int dead) {
    std::cout << "Day " << day << ": "
              << "Susceptible: " << susceptible << ", "
              << "Infected: " << infected << ", "
              << "Recovered: " << recovered << ", "
              << "Dead: " << dead << std::endl;
}

void print_end_statistics(int day, int recovered, int dead, int susceptible, clock_t timeStart, clock_t timeEnd) {
    double duration = double(timeEnd - timeStart) / CLOCKS_PER_SEC;
    std::cout << std::endl
              << "Day " << day << " (Years: " << round(day / 36.5f) / 10 << ") " << " ---------" << std::endl
              << "Still Susceptible: " << susceptible << std::endl
              << "Recovered: " << recovered << std::endl
              << "Dead: " << dead << std::endl
              << "Virus mutations: " << mutations << std::endl
              << std::endl
              << "Time taken to simulate: " << duration << std::endl;
}

int main() {
    clock_t timeStart = clock();

    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0.0, 1.0); // Distribution for infection chance

    // Allocate population dynamically on the heap
    Person* population = new Person[POPULATION_SIZE];
    for (long i = 0; i < POPULATION_SIZE; i++) {
        population[i].status = 0; // All are susceptible initially
        if (population[i].intelligence >= 98) {
            population[i].masked = true;
        }
    }

    // Infect initial population
    for (int i = 0; i < INITIAL_INFECTED; i++) {
        population[i].status = 1; // Set initial infected
    }

    // Simulate each day
    for (int day = 1; day <= DAYS; day++) {
        susceptible = 0, infected = 0, recovered = 0, dead = 0;

        simulate_day(population, POPULATION_SIZE, gen, dist);

        for (long i = 0; i < POPULATION_SIZE; i++) {
            if (population[i].status == 0) susceptible++;
            else if (population[i].status == 1) infected++;
            else if (population[i].status == 2) recovered++;
            else if (population[i].status == 3) dead++;
        }

        if (infected == 0) {
            std::cout << "Pandemic over!" << std::endl;
            clock_t timeEnd = clock();
            print_end_statistics(day, recovered, dead, susceptible, timeStart, timeEnd);
            delete[] population; // Free the dynamically allocated memory
            return 0;
        }

        print_statistics(day, susceptible, infected, recovered, dead);
    }

    std::cout << "Pandemic never finished. Please allocate more time." << std::endl;

    // Free the dynamically allocated memory
    delete[] population;

    return 0;
}