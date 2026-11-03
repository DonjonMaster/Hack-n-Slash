#include "cpe/simulation.hpp"
namespace cpe {
    // Gérer la requête, sa temporisation et les échecs.
void Navigator::Plan(const Level& level, Vec2 position,
                     Vec2 target, Algorithm algorithm) {
    
    if (requestCooldown > 0.0) return;
    bool changed = !hasTarget || Distance(target, lastTarget) > 0.75;
    bool retry = path.search.status != Status::Found;
    bool stoppedFar = Finished() && Distance(position, target) > 0.1;
    
    if (!changed && !retry && !stoppedFar) return;
    
    path = level.navmesh.FindPath(position, target, Algorithm::Dijkstra);
    next = path.points.size() > 1 ? 1 : path.points.size();
    
    lastTarget = target;
    hasTarget = true;
    requestCooldown = 0.3;
    ++requests;
}
    
    //Consomme les waypoints et applique un déplacement valide.
Vec2 Navigator::Follow(const Level& level, Vec2 position, double speed, double dt) {
    
    if (path.search.status != Status::Found || dt <= 0 || speed <= 0) return position;
    
    double budget = speed * dt;
    while (!Finished() && budget > 0)
    {
        Vec2 delta = path.points[next] - position;
        // Distance to the next point
        double distance = Length(delta);
        if (distance < 1e-8) {++next; continue;}
        // If nextpoint in range
        if (distance <= budget)
        {
            Vec2 displacement = path.points[next++];
            position = level.MoveSliding(position, displacement, level.agentRadius);
            budget -= distance;
        }
        // Else, go in the direction of the next point
        else
        {
            Vec2 displacement = position + delta * (budget / distance);
            position = level.MoveSliding(position, displacement, level.agentRadius);
            budget = 0;
        }
    }
    return position;
}
}
