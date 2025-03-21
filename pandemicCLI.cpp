#include <iostream>  // I/O
#include <random>    // RNG
#include <ctime>     // Time
#include <cmath>     // For M_PI and other math functions
#include <vector>    // For dynamic arrays
#include <omp.h>     // OpenMP for better parallelization

// Simulation settings
const long POPULATION_SIZE = 500000;
const int INITIAL_INFECTED = 12;
const int VIRUS_COMPLEXITY = 3;
const int DAYS = 25000;
const float INFECTION_CHANCE = 0.23f;
const float FATALITY = 0.01f;
const float RECOVERY_CHANCE = 0.05f;
const int maxInfection = 10;

// Counters for population status
long susceptible = POPULATION_SIZE - INITIAL_INFECTED, infected = INITIAL_INFECTED, recovered = 0, dead = 0;
int mutations = 0;
float vaccinePercent = 0;
float curePercent = 0;
float motivation = 1; // Start at 1 to avoid a negative log

bool vaccines = false;
bool cured = false;

double generate_intelligence_score() {
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    thread_local std::normal_distribution<> dist(100.0, 35.0);
    return dist(gen);
}

struct Person {
    enum Status { SUSCEPTIBLE = 0, INFECTED = 1, RECOVERED = 2, DEAD = 3 };
    Status status = SUSCEPTIBLE;
    bool masked = false;
    bool quarantined = false;
    int intelligence = generate_intelligence_score();
};

void virus_mutation(std::vector<Person>& population, float geneticDifference, std::mt19937& gen, std::uniform_real_distribution<>& dist) {
    mutations++;
    motivation += 0.5;
    vaccinePercent /= (dist(gen) * 100) + 0.0001;
    curePercent    /= (dist(gen) * 100) + 0.0001;
    std::cout << "Virus mutation. Genetic difference: " << geneticDifference << std::endl;
    
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < population.size(); i++) {
        thread_local std::random_device rd;
        thread_local std::mt19937 thread_gen(rd());
        thread_local std::uniform_real_distribution<> thread_dist(0.0, 1.0);
        if (thread_dist(thread_gen) > (1 - geneticDifference) && population[i].status == Person::RECOVERED) {
            population[i].status = Person::SUSCEPTIBLE;
        }
    }
}

void simulate_day(std::vector<Person>& population, std::mt19937& gen, std::uniform_real_distribution<>& dist) {
    float newVaccinePercent = vaccinePercent + (dist(gen) * std::log(motivation)) / VIRUS_COMPLEXITY;
    vaccinePercent = (newVaccinePercent > 100.0f) ? 100.0f : newVaccinePercent;
    if (vaccinePercent >= 100 && !vaccines) {
        std::cout << "Vaccines distributed!" << std::endl;
        vaccines = true;
    }

    const int chunk_size = 25000;
    #pragma omp parallel for schedule(dynamic, chunk_size)
    for (size_t i = 0; i < population.size(); i++) {
        thread_local std::random_device rd;
        thread_local std::mt19937 thread_gen(rd());
        thread_local std::uniform_real_distribution<> thread_dist(0.0, 1.0);

        if (vaccines && thread_dist(thread_gen) >= 0.5 + (population[i].intelligence/100)) {
            population[i].status = (population[i].status == Person::SUSCEPTIBLE) ? Person::RECOVERED : Person::SUSCEPTIBLE;
        }

        if (cured && thread_dist(thread_gen) >= 0.2 + (population[i].intelligence/100)) {
            population[i].status = (population[i].status == Person::INFECTED) ? Person::RECOVERED : Person::SUSCEPTIBLE;
        }

        if (population[i].intelligence >= 105 && population[i].status == Person::INFECTED) {
            population[i].quarantined = (thread_dist(thread_gen) >= 0.65);
            if (population[i].quarantined) {
                #pragma omp atomic
                motivation += 0.02;
            }
        } else {
            population[i].quarantined = false;
        }

        if (population[i].status == Person::INFECTED) {
            int infection_attempts = population[i].quarantined ? 1 : 
                                   (population[i].masked ? maxInfection / 2 : maxInfection * 2);
            
            for (int j = 0; j < infection_attempts; j++) {
                size_t randIndex = thread_gen() % population.size();
                if (population[randIndex].status == Person::SUSCEPTIBLE) {
                    float infection_probability = std::normal_distribution<>(INFECTION_CHANCE, 0.05)(thread_gen);
                    infection_probability = infection_probability < 0.0f ? 0.0f : (infection_probability > 1.0f ? 1.0f : infection_probability);
                    
                    if (thread_dist(thread_gen) < infection_probability) {
                        population[randIndex].status = Person::INFECTED;
                    }
                }
            }

            float rand = thread_dist(thread_gen);
            if (rand < RECOVERY_CHANCE) {
                population[i].status = Person::RECOVERED;
                #pragma omp atomic
                motivation += 0.05;  // Increased motivation boost for recovery
                if (thread_dist(thread_gen) < 0.1) {  // 10% chance for extra boost when multiple recoveries happen
                    #pragma omp atomic
                    motivation += 0.1;
                }
            } else if (rand < RECOVERY_CHANCE + FATALITY) {
                population[i].status = Person::DEAD;
                #pragma omp atomic
                motivation += (motivation > 5.0) ? -0.02 : 0.15;
            }
        }
    }

    curePercent += (dist(gen) * std::log(motivation)) / std::pow(VIRUS_COMPLEXITY, 2);
    if (curePercent >= 100 && !cured) {
        std::cout << "Virus cured!" << std::endl;
        cured = true;
    }

    if (dist(gen) < 0.005) {
        virus_mutation(population, dist(gen), gen, dist);
    }
}

void print_statistics(int day, int susceptible, int infected, int recovered, int dead, float motivation) {
    std::cout << "Day " << day << ": "
              << "Susceptible: " << susceptible << ", "
              << "Infected: " << infected << ", "
              << "Recovered: " << recovered << ", "
              << "Dead: " << dead << ", "
              << "Motivation (log): " << std::log(motivation) << std::endl;
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
              << "Time taken to simulate: " << duration << std::endl
              << std::endl
              << (vaccines ? "Vaccines have been distributed" : "Vaccines have not been distributed") << std::endl
              << (!vaccines ? ("Vaccine percent: " + std::to_string(vaccinePercent) + "%") : "") << std::endl
              << "Motivation (log): " << std::log(motivation) << std::endl;
}

int main() {
    omp_set_num_threads(omp_get_max_threads());
    clock_t timeStart = clock();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0.0, 1.0);

    std::vector<Person> population(POPULATION_SIZE);
    
    #pragma omp parallel for schedule(static)
    for (long i = 0; i < POPULATION_SIZE; i++) {
        population[i].masked = (population[i].intelligence >= 98);
    }

    for (int i = 0; i < INITIAL_INFECTED; i++) {
        population[i].status = Person::INFECTED;
    }

    for (int day = 1; day <= DAYS; day++) {
        susceptible = infected = recovered = dead = 0;

        simulate_day(population, gen, dist);

        #pragma omp parallel for reduction(+:susceptible,infected,recovered,dead) schedule(static)
        for (long i = 0; i < POPULATION_SIZE; i++) {
            switch (population[i].status) {
                case Person::SUSCEPTIBLE: susceptible++; break;
                case Person::INFECTED:    infected++;    break;
                case Person::RECOVERED:   recovered++;   break;
                case Person::DEAD:        dead++;        break;
            }
        }

        if (infected == 0) {
            std::cout << "Pandemic over!" << std::endl;
            clock_t timeEnd = clock();
            print_end_statistics(day, recovered, dead, susceptible, timeStart, timeEnd);
            return 0;
        }

        print_statistics(day, susceptible, infected, recovered, dead, motivation);
    }

    std::cout << "Pandemic never finished. Please allocate more time." << std::endl;
    return 0;
}