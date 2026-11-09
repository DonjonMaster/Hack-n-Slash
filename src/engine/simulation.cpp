#include "cpe/simulation.hpp"
#include <utility>

namespace cpe
{
    EnemyWave::EnemyWave(std::vector<Enemy>* enemies) : currentEnemies(enemies)
    {
    }

    void EnemyWave::Update(double dt)
    {
        elapsed += dt;
        if (enemySpawned != waves[currentWave].size())
        {
            if (elapsed >= cooldown)
            {
                elapsed = 0.0;
                currentEnemies->push_back(waves[currentWave][enemySpawned]);
                enemySpawned++;
            }
        }
        if (currentEnemies->empty())
        {
            currentWave++;
            if (currentWave >= waves.size())
            {
                //Victory()
            }
            else
            {
                *currentEnemies = waves[currentWave];
            }
        }
    }
Simulation::Simulation(Level level) : level(level),wave(&enemies){Reset();}
void Simulation::Reset() {
    player = Player{};
    player.position = level.playerSpawn;
    for (auto& x : player.statusEffect.conditions)
    {
        x.second = false;
    }
    player.statusEffect.conditions[None] = true;
    enemies.clear();
    elapsed = 0;
        for (const auto& spawn : level.spawns)
        {
            Enemy enemy;
            enemy.position = spawn.position;
            enemy.kind = spawn.kind;
            if (enemy.kind == EnemyKind::Light)
                SetLightEnemy(enemy);
            else if (enemy.kind == EnemyKind::Heavy)
                SetHeavyEnemy(enemy);
            else if (enemy.kind == EnemyKind::Archer)
                SetArcherEnemy(enemy);
            enemy.patrol = spawn.patrol;
            enemies.push_back(enemy);
        }
        for (const auto& spawnWave : level.waveSpawn)
        {
            std::vector<Enemy> enemyWave;
            for (const auto& spawn : spawnWave)
            {
                Enemy enemy;
                enemy.position = {
                    static_cast<double>(GetRandomValue(level.bounds.min.x, level.bounds.max.x)),
                    static_cast<double>(GetRandomValue(level.bounds.min.z, level.bounds.max.z))
                };
                while (level.navmesh.Locate(enemy.position) == -1)
                {
                    enemy.position = {
                        static_cast<double>(GetRandomValue(level.bounds.min.x, level.bounds.max.x)),
                        static_cast<double>(GetRandomValue(level.bounds.min.z, level.bounds.max.z))
                    };
                }
                
                enemy.kind = spawn.kind;
                if (enemy.kind == EnemyKind::Light)
                    SetLightEnemy(enemy);
                else if (enemy.kind == EnemyKind::Heavy)
                    SetHeavyEnemy(enemy);
                else if (enemy.kind == EnemyKind::Archer)
                    SetArcherEnemy(enemy);
                enemy.patrol = spawn.patrol;
                enemyWave.push_back(enemy);
                
            }
            wave.waves.push_back(enemyWave);
        }
}

void Simulation::SetAlgorithm(Algorithm value) {
    algorithm = value;
    for (auto& enemy : enemies) enemy.navigator.Invalidate();
}

std::size_t Simulation::RequestCount() const {
    std::size_t count = 0;
    for (const auto& enemy : enemies) count += enemy.navigator.requests;
    return count;
}

void Simulation::Update(Vec2 movement, bool actionPressed, double dt) {
    if (!std::isfinite(dt) || dt <= 0) return;
    dt = std::min(dt, 0.05);
    elapsed += dt;

    if (Finite(movement) && Length(movement) > 1e-8) {
        Vec2 direction = Normalized(movement);
        player.facing = direction;
        player.position = level.MoveSliding(
            player.position, direction * (3.5 * dt), level.agentRadius);
    }

    UpdateGameplay(actionPressed, dt);
    for (auto& enemy : enemies) 
        UpdateEnemy(enemy, dt);
    for (auto& drone : drones) UpdateDrone(drone, dt);
    for (auto& projectile : projectiles) UpdateProjectiles(projectile, dt);
        wave.Update(dt);
    
    auto it = std::find_if(drones.begin(), drones.end(), [](Drone& drone){return drone.deathDuration == 0;});
    if (it != drones.end())
        drones.erase(it);
        
        
}

void Simulation::SetLightEnemy(Enemy& enemy)
{
    enemy.hp = 20;
    enemy.damage = 5;
    enemy.speed = 3.0f;
    enemy.attackRange = 1.0f;
    enemy.windupDuration = 1.5f;
    enemy.recoverDuration = 0.8f;
    
    enemy.detectRange = 2.5f;
    enemy.loseRange = 5.0f;
}

void Simulation::SetHeavyEnemy(Enemy& enemy)
{
    enemy.hp = 60;
    enemy.damage = 10;
    enemy.speed = 1.5f;
    enemy.attackRange = 1.5f;
    enemy.windupDuration = 2.25f;
    enemy.recoverDuration = 1.5f;
    
    enemy.detectRange = 3.0f;
    enemy.loseRange = 5.0f;
}

void Simulation::SetArcherEnemy(Enemy& enemy)
{
    enemy.hp = 10;
    enemy.damage = 5;
    enemy.speed = 2.0f;
    enemy.attackRange = 5.0f;
    enemy.windupDuration = 1.5f;
    enemy.recoverDuration = 1.5f;
    
    enemy.detectRange = 8.0f;
    enemy.loseRange = 10.0f;
}
}
