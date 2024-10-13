# Object Oriented vs Data Oriented Design for Cache Efficiency

## Premise

We have a N bodies with random (double precision) mass floating in 2-dimensional space at random (discrete) positions. We need to calulate accellerations, velocities and new positions for each of the bodies over a thousand 1-second epochs, 
taking into account the effect of gravity between the objects. It is not considered acceptable to use an approximation method such as [Barnes-Hut] (https://en.wikipedia.org/wiki/Barnes%E2%80%93Hut_simulation), you must instead perform
an N^2 iteration for each epoch.

## Experiment Design

Just to ensure both methods actually calculate the same state, we'll use `rand()` from `<cmath>` to generate initial values for our bodies' mass, position, velocity etc. This will give use deterministic generation of initial values and so the outcome of the system should also be deterministic. We can then perform a checksum over the final state (e.g. sum all the velocities of all the bodies) to validate that both solutions got the same answer.

## Object Oriented Approach

I think that the intuitive object oriented design for this system is to create a class for a body which stores mass & velocity of the object. The initial code looked like this:

shared.hpp:

```
#pragma once

#include <cmath>
#include <cstdlib>

#define NUM_OBJECTS 1000
#define NUM_EPOCHS 1000
#define MAX_SPAWN_DISTANCE 10000

#define G (6.67430e-11)

auto calculate_gravitational_force(double m1, double m2, double distance) -> double
{
    return (G * m1 * m2) / (pow(distance, 2));
}

auto generate_position()
{
    return rand() % 10000;
}

```
oo.cpp:
```
#include <vector>
#include <iostream>
#include <cstdint>

#include "shared.hpp"

static int next_id = 0;

class object
{
public:
    object()
    {
        id = next_id;
        next_id++;
        position = { generate_position(), generate_position() };
        velocity = { 0, 0 };
        mass = 0;
    }

    uint32_t id;
    std::pair<double, double> position, velocity;
    double mass;
};


std::pair<double, double> operator-(const std::pair<double, double> lhs,
                    const std::pair<double, double>& rhs)
{
    return std::pair(lhs.first - rhs.first, lhs.second - rhs.second);
}

auto operator+(const std::pair<double, double>& lhs,
                    const std::pair<double, double>& rhs)
{
    return std::pair(lhs.first + rhs.first, lhs.second + rhs.second);
}

auto operator*(const std::pair<double, double>& lhs, double rhs)
{
    return std::pair(lhs.first * rhs, lhs.second * rhs);
}

class space
{
public:
    space() : objects(std::vector(NUM_OBJECTS, object())) {}

    auto calculate() -> void
    {
        for (auto& object: objects)
        {
            // Calculate force applied to object
            // Calculate accelleration, update velocity
            // Apply velocity
            auto force = std::pair<double, double>();
            auto& [fx, fy] = force;

            for (const auto& other: objects)
            {
                if (object.id == other.id)
                    continue;

                auto [dx, dy] = object.position - (other).position;

                fx += calculate_gravitational_force(object.mass, other.mass, dx);
                fy += calculate_gravitational_force(object.mass, other.mass, dy);
            }

            auto accel = force * object.mass;

            // Apply accelleration for one unit-time:
            object.velocity = object.velocity + accel;
            
            // Apply velocity for one unit-time
            object.position = object.position + object.velocity;
        }
    }

    auto checksum() const -> double
    {
        double ret = 0;

        for (const auto& object: objects)
        {
            const auto& [px, py] = object.position;
            const auto& [vx, vy] = object.velocity;
            ret+= px + py + vx + vy;
        }

        return ret;
    }

private:
    std::vector<object> objects;
};

int main()
{
    space s;

    for (int i = 0; i < NUM_EPOCHS; i++)
        s.calculate();

    std::cout << s.checksum() << "\n";

    return 0;
}
```

Taking an initial profile using perf, it looks like this solution has some issues with frontend stalls:
```
 Performance counter stats for './src/oo':

          7,672.82 msec task-clock                       #    1.000 CPUs utilized             
                27      context-switches                 #    3.519 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
               142      page-faults                      #   18.507 /sec                      
    31,703,786,692      cycles                           #    4.132 GHz                         (71.43%)
     8,053,939,186      stalled-cycles-frontend          #   25.40% frontend cycles idle        (71.43%)
    75,567,425,677      instructions                     #    2.38  insn per cycle            
                                                  #    0.11  stalled cycles per insn     (71.43%)
    12,080,856,691      branches                         #    1.575 G/sec                       (71.43%)
         1,265,581      branch-misses                    #    0.01% of all branches             (71.43%)
    46,438,238,629      L1-dcache-loads                  #    6.052 G/sec                       (71.42%)
       757,364,030      L1-dcache-load-misses            #    1.63% of all L1-dcache accesses   (71.42%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

       7.673854031 seconds time elapsed

       7.671570000 seconds user
       0.001999000 seconds sys
```

But we are easily able to get a good reduction in frontend stalls by simply inlining our `calculate_gravitational_force()` function. I actually had to use `[[gnu::always_inline]]` to force g++ to inline it, this alone reduces iCache misses by ~14%:
```
     3,038,664,962      stalled-cycles-frontend          #   11.82% frontend cycles idle        (71.42%)
```

Removing all uses of `std::vector` and `std::pair` and replacing that with `std::array` and:
```
template <typename T, typename U>
struct pair
{
    T first;
    U second;
};
```
does reduce file size from ~35kb to ~17kb and we see a dramatic drop in frontend stalls, but we've gone backwards! Our program runs slower!
```
         21,984.50 msec task-clock                       #    1.000 CPUs utilized             
               122      context-switches                 #    5.549 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
                77      page-faults                      #    3.502 /sec                      
    87,599,663,427      cycles                           #    3.985 GHz                         (71.43%)
     5,065,009,787      stalled-cycles-frontend          #    5.78% frontend cycles idle        (71.43%)
   241,239,746,233      instructions                     #    2.75  insn per cycle            
                                                  #    0.02  stalled cycles per insn     (71.43%)
    39,019,990,137      branches                         #    1.775 G/sec                       (71.43%)
         2,970,463      branch-misses                    #    0.01% of all branches             (71.43%)
    99,169,076,569      L1-dcache-loads                  #    4.511 G/sec                       (71.43%)
       749,169,875      L1-dcache-load-misses            #    0.76% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      21.986213315 seconds time elapsed

      21.983902000 seconds user
       0.000999000 seconds sys
```
