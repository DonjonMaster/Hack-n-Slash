#include <raylib.h>

#include "cpe/simulation.hpp"
namespace cpe {
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
