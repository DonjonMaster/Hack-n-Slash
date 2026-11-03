#include "cpe/simulation.hpp"
namespace cpe {
void Simulation::UpdateEnemy(Enemy& enemy, double dt) {
    // Perception, transitions, décisions et appel du Navigator.
    
    if (enemy.state == State::Dead)return;
    
    if (enemy.state == State::Waiting) enemy.state = State::Idle;
    
    if (enemy.hp <= 0)
    {
        enemy.state = State::Dead;
        enemy.navigator.Invalidate();
        return;
    }
    
    elapsed = std::max(0.0, elapsed-dt);
    enemy.navigator.requestCooldown = std::max(0.0, enemy.navigator.requestCooldown-dt);
    
    const double distance = Distance(enemy.position, player.position);
    if (player.hp <= 0 || distance > enemy.loseRange)
    {
        enemy.state = State::Idle;
        enemy.navigator.Invalidate();
        elapsed = 0;
        return;
    }
    
    switch (enemy.state)
    {
    case State::Idle:
        if (distance <= enemy.detectRange)
            enemy.state = State::Chase;
        break;
    case State::Chase:
        if (distance <= enemy.attackRange && level.navmesh.SegmentInside(enemy.position, player.position))
        {
            enemy.state = State::Windup;
            elapsed = enemy.windupDuration;
        }
        else
        {
            enemy.navigator.Plan(level, enemy.position, player.position, algorithm);
            enemy.position = enemy.navigator.Follow(level, enemy.position, enemy.speed, dt);
        }
        break;
    case State::Windup:
        if (elapsed <= 0)
        {
            if (distance <= enemy.attackRange && level.navmesh.SegmentInside(enemy.position, player.position))
                player.hp = std::max(0, player.hp - enemy.damage);
            enemy.state = State::Recover;
            elapsed = enemy.recoverDuration;
        }
        break;
    case State::Recover:
        if (elapsed <= 0)
            enemy.state = State::Chase;
        break;
    case State::Dead:
        break;
    }
    
}
}
