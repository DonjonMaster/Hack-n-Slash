#include "cpe/simulation.hpp"

namespace cpe
{
    void Simulation::UpdateDrone(Drone& drone, double dt)
    {
        switch (drone.state)
        {
        case DroneState::Moving:
            {
                if (drone.movingDuration > 0)
                {
                    drone.navigator.Plan(level, drone.position, drone.target, algorithm);
                    drone.position = drone.navigator.Follow(level, drone.position, drone.speed, dt);
                    drone.movingDuration = std::max(0.0,drone.movingDuration - dt);
                }
                else
                {
                    drone.state = DroneState::Exploding;
                }
                    
                break;
            }
        case DroneState::Exploding:
            {
                if (drone.explosionDuration > 0)
                {
                    drone.explosionDuration = std::max(0.0, drone.explosionDuration - dt);
                }
                else
                {
                    for (auto& enemy : enemies)
                    {
                        if (Distance(drone.position, enemy.position) <= drone.explosionRadius)
                        {
                            enemy.hp = std::max(0, enemy.hp - drone.damage);
                            if (enemy.hp == 0 && enemy.state != State::Dead)
                                PlaySound(player.deathSound);
                            else if (enemy.hp != 0)
                                PlaySound(player.enemyHitSound);
                        }
                    }
                    if (Distance(drone.position, player.position) <= drone.explosionRadius)
                    {
                        player.TakeDamage(drone.damage, dt, None);
                    }
                    drone.state = DroneState::Exploded;
                }
                break;
            }
        case DroneState::Exploded:
            {
                if (drone.exploseDuration == 0)
                    drone.state = DroneState::Death;
                
                else
                drone.exploseDuration = std::max(0.0, drone.exploseDuration - dt);
                break;
            }
        case DroneState::Death:
            {
                drone.deathDuration = std::max(0.0, drone.deathDuration - dt); 
                break;
            }
        }
    }
}
