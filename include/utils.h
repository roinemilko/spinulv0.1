#ifndef UTILS_H
#define UTILS_H

#include "raylib.h"
#include "raymath.h"
#include <vector>


// simulation structs
struct Params {
    float scale = 5.0f; // parameter to set the scale of the animation
    Color particle_color = MAROON;
    Color spin_color = BLUE;
    int n_of_particles = 10; // the number of sites in the chain
    float dt_ps = 0.05f; // lenght of time step in integrator (picoseconds)
    float J1 = -1.6f; // nearest neighbour coupling factor (meV)
    float J2 = 0.44f; // next-nearest neighbour coupling factor (meV)
    float external_field = 0; // mT
    float external_field_radius = 5.0f;
    float ext_field_pulse_lenght = 5;  // picoseconds
    bool ext_field_on = false;
    const float hbar = 0.6582; // meV * picoseconds
    float damping = 0.4f; // the damping coefficient
    float gm_ratio = 2;
    float bohr_magneton = 0.05788;
    int sponge_width = 5; // the width of the "sponge" at the ends in number of sites
    float energy_resolution = 0.1f;
};

struct Particle {
    Vector3 pos;
    Vector3 spin;
};

// util functons
void DrawArrow(Particle* particle, Params* params);
void InitParticles(std::vector<Particle>& particles, Params* params);
void DrawAxes(Params* params);
void DrawFieldVisual(Params* params);

// physics
Vector3 CalculateH_eff(int i, const std::vector<Particle>& particles, Params* params);
void MinimizeEnergy(std::vector<Particle>& particles, Params* params);
float getTotalEnergy(const std::vector<Particle>& particles, Params* params);

#endif