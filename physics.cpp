#include "utils.h"
#include <vector>
#include "raylib.h"
#include "math.h"

// CALCULATE THE EFFECTIVE MAGNETIC FIELD STRENGHT OF A SITE
// PARAMS: (index of the site), (reference to a vector of the sites), (pointer to simulation params)
// RETURNS: the effective field strenght as a vector
Vector3 CalculateH_eff(int i, const std::vector<Particle>& particles, Params* params) {

    // Circular handling of the edges
    auto handle_edges = [&](int idx) {
        int N = params->n_of_particles;
        int wrap = (idx % N + N) % N;
        return particles[wrap].spin;
    };

    Vector3 H = Vector3Zero();

    // Nearest neighbour term
    Vector3 J1 = Vector3Add(handle_edges(i-1), handle_edges(i+1));
    H = Vector3Add(H, Vector3Scale(J1, -params->J1));

    // Next nearest neighbour term
    Vector3 J2 = Vector3Add(handle_edges(i-2), handle_edges(i+2));
    H = Vector3Add(H, Vector3Scale(J2, -params->J2));

    // Zeeman term
    if (particles[i].pos.x >= (50.0f - params->external_field_radius) && particles[i].pos.x <= (50.0f + params->external_field_radius)) {
        Vector3 zeemann = Vector3Scale((Vector3) {0.0f, 0.0f, params->external_field}, params->gm_ratio * params->bohr_magneton);
        H = Vector3Add(H, zeemann);
    }

    return H;
}
// INTEGRATOR: UPDATE THE SPINS TO THE NEXT TIME STEP
// PARAMS: (reference to vector of the sites), (pointer to simulation params)
void MinimizeEnergy(std::vector<Particle>& particles, Params* params) {
    float damping = params->damping;
    float gamma = 1 / params->hbar;
    float dt = params->dt_ps;
    int N = params->n_of_particles;

    std::vector<Vector3> new_spins(N);

    // Calculate new spins after time step
    for (int i = 0; i < N; i++) {

        // Linear "sponge" damping at the ends of the chain to reduce artificial constructive interference
        float local_damping;
        int sponge_width = params->sponge_width;
        float max_damping = 0.5f;

        if (i <= sponge_width) {
            float how_close = ((float) i) / ((float) sponge_width);
            local_damping = damping + (max_damping * (1-how_close));
        }
        else if (i >= N - sponge_width) {
            float how_close = ((float) (N - i)) / ((float) sponge_width);
            local_damping = damping + (max_damping * (1-how_close));
        }

        Vector3 H = CalculateH_eff(i, particles, params);
        Vector3 S = particles[i].spin;

        // Precession term
        Vector3 torque = Vector3Scale(Vector3CrossProduct(S, H), -gamma);

        // Damping term
        Vector3 CP = Vector3CrossProduct(S, H);
        Vector3 gilbert_damping = Vector3Scale(
            Vector3CrossProduct(S, CP),
            -gamma * local_damping
        );

        Vector3 dS = Vector3Add(torque, gilbert_damping);
        new_spins[i] = Vector3Normalize(Vector3Add(S, Vector3Scale(dS, dt)));
    }
    // Update spins
    for (int i = 0; i < N; i++) { particles[i].spin = new_spins[i]; }
}

// CALCULATE TOTAL ENERGY OF THE SYSTEM
// PARAMS: (reference to a vector of the sites), (pointer to the simulation params)
float getTotalEnergy(const std::vector<Particle>& particles, Params* params) {
    float result = 0.0f;
    int N = params->n_of_particles;

    auto handle_edges = [&](int idx) {
        int wrap = (idx % N + N) % N;
        return particles[wrap].spin;
    };

    for (int i = 0; i < N; i++) {
        Vector3 S = particles[i].spin;

        result += params->J1 * Vector3DotProduct(S, handle_edges(i + 1));
        result += params->J2 * Vector3DotProduct(S, handle_edges(i + 2));
        result -= params->external_field * S.z;
    }
    return result;
}