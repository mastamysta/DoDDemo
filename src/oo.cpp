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
