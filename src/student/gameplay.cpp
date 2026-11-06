#include "cpe/simulation.hpp"

namespace cpe
{
    bool Simulation::MouseGround(Camera3D camera, Vec2& result)
    {
        Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
        if (std::abs(ray.direction.y) < 1e-6f) return false;
        float t = -ray.position.y / ray.direction.y;
        if (t < 0) return false;
        result = {ray.position.x + ray.direction.x * t, ray.position.z + ray.direction.z * t};
        return true;
    }

    void Simulation::UpdateGameplay(bool actionPressed, double dt)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            MouseGround(*camera, player.target);
        }
        player.attackCooldown = std::max(0.0, player.attackCooldown - dt);
        if (actionPressed)
        {
            if (player.attackCooldown == 0)
            {
                Drone drone;
                drone.position = player.position;
                drone.target = player.target;
                drones.push_back(drone);
                player.attackCooldown = player.attackCooldownMax;
            }
        }
        player.statusEffect.ApplyEffect(player.hp, dt);
    }

    void Player::TakeDamage(int damage, double dt, Effects effect)
    {
        hp = std::max(0, hp - damage);
        statusEffect.ApplyStatus(effect);
    }

    void StatusEffect::ApplyStatus(Effects effect)
    {
        if (effect == Fire)
        {
            conditions[None] = false;
            conditions[effect] = true;
            fireTimer = 1.2;
            fireCount = 4;
        }
        else if (effect == Poisonned)
        {
            conditions[None] = false;
            conditions[effect] = true;
            poisonTimer = 1;
            poisonCount = 5;
        }
    }

    void StatusEffect::ApplyEffect(int& hp, double dt)
    {
        if (conditions[None])
            return;

        if (fireTimer <= 0 && conditions[Fire])
        {
            {
                if (fireCount >= 1)
                {
                    hp = std::max(0, hp - fireDamage);
                    --fireCount;
                    fireTimer = 1.2;
                }
                else
                {
                    conditions[Fire] = false;
                    conditions[None] = true;
                }
            }
        }
        else if (poisonTimer <= 0 && conditions[Poisonned])
        {
            // Poison Effect
            if (conditions[Poisonned])
            {
                if (poisonCount >= 1)
                {
                    hp = std::max(0, hp - poisonDamage);
                    --poisonCount;
                    poisonTimer = 1;
                }
                else
                {
                    conditions[Poisonned] = false;
                    conditions[None] = true;
                }
            }
        }
        fireTimer -= dt;
        poisonTimer -= dt;
    }
}
