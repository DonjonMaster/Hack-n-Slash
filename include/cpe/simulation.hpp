#pragma once
#include "level.hpp"
#include <cstddef>
#include <raylib.h>


namespace cpe {
    
   
    struct Player
    {
        Vec2 position;
        Vec2 facing{0, -1};
        int hp = 2000;
        double attackCooldown = 1.0;
        double attackCooldownMax = 1.0;
        Vec2 target = {0,0};
    };

// Ce composant expose les données au debug ; le suivi est à implémenter.
struct Navigator {
    double speed = 2.0;
    Path path;
    std::size_t next = 0;
    double requestCooldown = 0.0;
    Vec2 lastTarget;
    bool hasTarget = false;
    std::size_t requests = 0;
    
    void Invalidate();
    bool Finished() const;
    void Plan(const Level& level, Vec2 position, Vec2 target, Algorithm algorithm);
    Vec2 Follow(const Level& level, Vec2 position, double speed, double dt);
};
    template<typename StateType>
    class Agent
    {
    public:
        Vec2 position;
        float speed = 2.0;
        int damage = 2;
        Navigator navigator;
        StateType state;
    };
    
    enum class DroneState { Moving, Exploding, Exploded };
    class Drone : public Agent<DroneState>
    {
    public:
        DroneState state = DroneState::Moving;
        Vec2 target;
        double movingDuration = 2.0, explosionDuration = 2.0;
        double explosionRadius = 0.50;
    };
    
    
enum class State { Waiting, Idle, Chase, Windup, Recover, Dead };
const char* Name(State value);

// Entités de démonstration immobiles ; aucun comportement n'est fourni.
// Ajoutez ici les données nécessaires à votre propre IA et votre combat.
class Enemy : public Agent<State> {
public:
    int hp = 20;
    bool flashing = false;
    double detectRange = 2.0, loseRange=10.0, attackRange=0.65;
    double windupDuration=1.5, recoverDuration=0.8, flashingCooldown = 0.25f, flashingCooldownMax = 0.25f;
    std::vector<Vec2> patrol;
    int nextPatrolPoint = 0;
    EnemyKind kind = EnemyKind::Light;
    State state = State::Waiting;
};
class Simulation {
public:
    explicit Simulation(Level level);
    Level level;
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Drone> drones;
    Algorithm algorithm = Algorithm::AStar;
    double elapsed = 0;
    Camera3D* camera;

    // Fourni : déplacement, collisions, boucle et appels des points d'extension.
    void Update(Vec2 movement, bool actionPressed, double dt);
    void SetLightEnemy(Enemy& enemy);
    void SetHeavyEnemy(Enemy& enemy);
    void SetArcherEnemy(Enemy& enemy);
    void Reset();
    void SetAlgorithm(Algorithm value);
    std::size_t RequestCount() const;
    static bool MouseGround(Camera3D camera, cpe::Vec2& result);

private:
    // À réaliser dans src/student/enemy_ai.cpp et gameplay.cpp.
    void UpdateEnemy(Enemy& enemy, double dt);
    void UpdateDrone(Drone& drone, double dt);
    void UpdateGameplay(bool actionPressed, double dt);
};
}
