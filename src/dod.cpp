#include <array>
#include <iostream>
#include <cstdint>

#include "shared.hpp"

class space
{
public:
    space()
    {
        for (int i = 0; i < NUM_OBJECTS; i++)
        {
            px[i] = generate_position();
            py[i] = generate_position();
            m[i] = generate_position();
        }
    }

    auto calculate() -> void
    {
        for (int i = 0; i < NUM_OBJECTS; i++)
        {
            // Calculate force applied to object
            // Calculate accelleration, update velocity
            // Apply velocity
            auto force = std::pair<double, double>();
            auto& [fx, fy] = force;

            for (int j = 0; j < NUM_OBJECTS; j++)
            {
                if (i == j)
                    continue;

                auto dx = px[i] - px[j], dy = py[i] - py[j];

                fx += calculate_gravitational_force(m[i], m[j], dx);
                fy += calculate_gravitational_force(m[i], m[j], dy);
            }

            auto ax = fx / m[i], ay = fy / m[i];

            // Apply accelleration for one unit-time:
            vx[i] += ax;
            vy[i] += ay;
            
            // Apply velocity for one unit-time
            px[i] += vx[i];
            py[i] += vy[i];
        }
    }

    auto checksum() -> double
    {
        double ret = 0;

        for (int i = 0; i < NUM_OBJECTS; i++)
        {
            ret+= px[i] + py[i] + vx[i] + vy[i];
        }

        return ret;
    }

private:
    std::array<double, NUM_OBJECTS> vx, vy, px, py, m;
};

int main()
{
    space s;

    for (int i = 0; i < NUM_EPOCHS; i++)
        s.calculate();

    std::cout << s.checksum() << "\n";

    return 0;
}
