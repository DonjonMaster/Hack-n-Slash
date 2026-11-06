#include "cpe/simulation.hpp"
namespace cpe {
    
    void Simulation::UpdateProjectiles(Projectile& projectile, double dt)
    {
        projectile.position = projectile.position + (projectile.direction * (projectile.speed * dt));
        
        auto i = std::find_if(projectiles.begin(), projectiles.end(), 
            [this](const Projectile& projectile){return Distance(player.position, projectile.position) <= 1 - level.agentRadius;});
        if (i != projectiles.end())
        {
            // TODO : Remplacer par un TakeDamage()
            player.TakeDamage(projectile.damage, dt, Poisonned);
            projectiles.erase(i);
        }
        
        auto it = std::find_if(projectiles.begin(), projectiles.end(), 
            [this](const Projectile& projectile){return level.navmesh.Locate(projectile.position) == -1;});
        if (it != projectiles.end())
        {
            projectiles.erase(it);
        }
    }
    
    Projectile Enemy::UseBow(Vec2 target)
    {
        Projectile projectile;
        auto distance = Distance(target, position);
        projectile.direction = (target - position) * (1/distance);
        projectile.position = position;
        projectile.damage = damage;
        return projectile;
    }

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
    enemy.flashingCooldown = std::max(0.0, enemy.flashingCooldown-dt);
    
    const double distance = Distance(enemy.position, player.position);
    if (enemy.state != State::Idle && (player.hp <= 0 || distance > enemy.loseRange))
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
        else
        {
            if (enemy.patrol.size() > 0)
            {
                enemy.navigator.Plan(level, enemy.position, enemy.patrol[enemy.nextPatrolPoint], algorithm);
                enemy.position = enemy.navigator.Follow(level, enemy.position, enemy.speed, dt);
                if (enemy.navigator.Finished())
                    enemy.nextPatrolPoint =  (enemy.nextPatrolPoint + 1) % enemy.patrol.size();
            }
        }
        break;
    case State::Chase:
        if (distance <= enemy.attackRange && level.navmesh.SegmentInside(enemy.position, player.position))
        {
            enemy.state = State::Windup;
            elapsed = enemy.windupDuration;
            enemy.flashing = true;
            enemy.flashingCooldown = enemy.flashingCooldownMax;
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
            if (enemy.kind == EnemyKind::Archer)
            {
                projectiles.push_back(enemy.UseBow(player.position));
            }
            else if (distance <= enemy.attackRange && level.navmesh.SegmentInside(enemy.position, player.position))
            {
                if (enemy.kind == EnemyKind::Light)
                    player.TakeDamage(enemy.damage, dt, None);
                else if (enemy.kind == EnemyKind::Heavy)
                    player.TakeDamage(enemy.damage, dt, Fire);
            }
            enemy.state = State::Recover;
            elapsed = enemy.recoverDuration;
            enemy.flashingCooldown = enemy.flashingCooldownMax;
            enemy.flashing = false;
        }
        else
        {
            if (enemy.flashingCooldown <= 0)
            {
                enemy.flashing = !enemy.flashing;
                enemy.flashingCooldown = enemy.flashingCooldownMax;
            }
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
