#pragma once

#include <cmath>
#include <cstdlib>

#define NUM_OBJECTS 1000
#define NUM_EPOCHS 1000
#define MAX_SPAWN_DISTANCE 10000

#define G (6.67430e-11)

[[gnu::always_inline]]
auto calculate_gravitational_force(double m1, double m2, double distance) -> double
{
    return (G * m1 * m2) / (pow(distance, 2));
}

auto generate_position()
{
    return rand() % 10000;
}
