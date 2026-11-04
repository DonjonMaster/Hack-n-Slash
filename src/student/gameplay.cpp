#include "cpe/simulation.hpp"
namespace cpe {
    bool Simulation::MouseGround(Camera3D camera, cpe::Vec2& result)
    {
        Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
        if (std::abs(ray.direction.y) < 1e-6f) return false;
        float t = -ray.position.y / ray.direction.y;
        if (t < 0) return false;
        result = {ray.position.x + ray.direction.x * t, ray.position.z + ray.direction.z * t};
        return true;
    }
void Simulation::UpdateGameplay(bool actionPressed, double dt) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        MouseGround(*camera, player.target);
    }
    if (actionPressed)
    {
        Drone drone;
        drone.position = player.position;
        drone.target = player.target;
        drones.push_back(drone);
    }
}
}
