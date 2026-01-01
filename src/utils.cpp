#include "raylib.h"
#include "raymath.h"
#include "utils.h"
#include <cstdio>
#include <random>


// DRAW THE SPIN ARROW IN THE ANIMATION
// PARAMS: (pointer to the particle to be drawn), (pointer to simulation params)
void DrawArrow(Particle* particle, Params* params) {

    Vector3 end_pos = Vector3Add(particle->pos, Vector3Scale(particle->spin, params->scale));

    DrawPoint3D(particle->pos, params->particle_color);
    DrawLine3D(particle->pos, end_pos, params->spin_color);
    DrawCylinderEx(end_pos, Vector3Add(end_pos, Vector3Scale(particle->spin, 0.3f * params->scale)), 0.15f * params->scale, 0.0f, 8, params->spin_color);
}

// INITIALIZE THE PARTICLES
// PARAMS: (reference to a vector of the particles), (pointer to simulation params)
void InitParticles(std::vector<Particle>& particles, Params* params) {
    particles.clear();
    float distance = 100.0f / params->n_of_particles;

    std::random_device rd;
    std::mt19937 gen(rd());

    double mean_theta = 3.14;
    double sigma_theta = 1.5;
    std::normal_distribution<double> distribution_theta(mean_theta, sigma_theta);


    for (int i = 0; i < params->n_of_particles; i++) {
        Vector3 position = (Vector3) { (float)i * distance, 0, 0 };
        float theta = (float) distribution_theta(gen);
        Vector3 spin = (Vector3) {
            cos(theta),
            sin(theta),
            0
        };
        Particle particle = (Particle) {position, spin};
        particles.push_back(particle);
    }

}

// DRAW X,Y,Z AXES IN THE ANIMATION
// PARAMS: (pointer to simulation params)
void DrawAxes(Params* params) {
    Vector3 origin = (Vector3) {0, 2, -2};
	DrawLine3D(
		origin,
		Vector3Add(origin, (Vector3) {params->scale, 0, 0}),
		RED
	);
	DrawLine3D(
		origin,
		Vector3Add(origin, (Vector3) {0, params->scale, 0}),
        GREEN
	);
	DrawLine3D(
		(Vector3) {0, 0, 0},
		Vector3Add(origin, (Vector3) {0, 0, params->scale}),
		YELLOW
	);
}

// DRAW A SIMPLE VISUAL INDICATOR FOR THE APPLIED EXTERNAL FIELD
// PARAMS: (pointer to simulation params)
void DrawFieldVisual(Params* params) {
    DrawCylinderEx(
        (Vector3) {50.0f, -5.0f, 0.0f},
        (Vector3) {50.0f, 5.0f, 0.0f},
        params->external_field_radius,
        params->external_field_radius,
        8,
        Fade(GREEN, params->external_field / 15.0f)
    );
    Vector3 arrow_start = (Vector3) { 45.0f, 7.0f, 0.0f };
    Vector3 arrow_end = (Vector3) { 52.5f, 7.0f, 0.0f };
    DrawLine3D(arrow_start, arrow_end, GREEN);

    Vector3 tip_start = (Vector3) { 52.5f, 7.0f, 0.0f };
    Vector3 tip_end = (Vector3) { 55.0f, 7.0f, 0.0f };
    DrawCylinderEx(tip_start, tip_end, 1.0f, 0.0f, 8, GREEN);
}