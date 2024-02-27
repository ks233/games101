#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL
{

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.

        for (int i = 0; i < num_nodes; i++)
        {
            Vector2D pos = start + (end - start) * (float)i / num_nodes;
            std::cout << pos << std::endl;
            masses.push_back(new Mass(pos, node_mass, false));
        }
        for (int i = 0; i < num_nodes - 1; i++)
        {
            springs.push_back(new Spring(masses[i], masses[i + 1], k));
        }
        for (auto &i : pinned_nodes)
        {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D b2a = s->m2->position - s->m1->position;
            float distance = sqrt(dot(b2a, b2a));
            Vector2D f = s->k * b2a / distance * (distance - s->rest_length);
            s->m1->forces += f;
            s->m2->forces -= f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity * m->mass;
                // TODO (Part 2): Add global damping
                m->forces += -0.01 * m->velocity;
                Vector2D v = m->velocity;
                m->velocity += m->forces / m->mass * delta_t;
                // m->position += v * delta_t; // 显式欧拉
                m->position += m->velocity * delta_t; // 隐式欧拉
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D b2a = s->m2->position - s->m1->position;
            float distance = sqrt(dot(b2a, b2a));
            Vector2D f = s->k * b2a / distance * (distance - s->rest_length);
            s->m1->forces += f;
            s->m2->forces -= f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                Vector2D a = m->forces / m->mass + gravity;
                // TODO (Part 3.1): Set the new position of the rope mass
                m->position = temp_position + (1-0.00005) * (temp_position - m->last_position) + a * delta_t * delta_t;
                // TODO (Part 4): Add global Verlet damping
                m->last_position = temp_position;
            }
            m->forces = Vector2D(0, 0);
        }
    }
}
