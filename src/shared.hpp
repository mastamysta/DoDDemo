#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>

#define NUM_OBJECTS 1000
#define NUM_EPOCHS 100
#define MAX_SPAWN_DISTANCE 100

#define G (6.67430e-11)

auto calculate_gravitational_force(double m1, double m2, double distance) -> double
{
    if (distance == 0)
        return 0;

    return (G * m1 * m2) / (pow(distance, 2));
}

auto generate_position() -> double
{
    return (rand() % MAX_SPAWN_DISTANCE) + 1;
}
