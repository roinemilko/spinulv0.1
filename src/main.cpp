#include <cstdio>
#include <vector>
#include <cstdlib>
#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "utils.h"
#include <math.h>


int main(void) {
    InitWindow(1600, 400, "1D chain spin wave simulation");
    rlImGuiSetup(true);

	// init variables
    Params params;
    bool start = false;
    bool found_ground_state = false;
    float pulse_start_time = 0.0f;
    std::vector<Particle> particles;
    std::vector<float> energyPlot;
    std::vector<float> spinZPlot;

	// init camera for animation
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 50.0f, 0.0f, -35.0f };
    camera.target = (Vector3){ 50.0f, 5.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    SetTargetFPS(60);
    Shader lightShader = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");

    while (!WindowShouldClose())
    {
        BeginDrawing();

            if (start) {
                // Draw the animation
                ClearBackground(DARKGRAY);
                BeginMode3D(camera);
                BeginShaderMode(lightShader);
                    for (int i = 0; i < params.n_of_particles; i++) {
                        DrawArrow(&particles[i], &params);
                    }
					DrawAxes(&params);
                    if (params.ext_field_on) {DrawFieldVisual(&params);}
                EndShaderMode();
                EndMode3D();
                MinimizeEnergy(particles, &params);

                if (found_ground_state) {params.current_time += params.dt_ps;}
            }

            // Create the GUI
            rlImGuiBegin();

            ImGui::Begin("Settings");
                ImGui::SliderFloat("Scale", &params.scale, 0.2f, 20.0f);
                ImGui::SliderFloat("J1", &params.J1, -5.0f, 5.0f);
                ImGui::SliderFloat("J2", &params.J2, -5.0f, 5.0f);
                ImGui::SliderFloat("External field", &params.external_field, 0.0f, 15.0f);
                ImGui::SliderFloat("Pulse lenght:", &params.ext_field_pulse_lenght, 1.0f, 5.0f);
                ImGui::SliderFloat("Field radius", &params.external_field_radius, 5.0f, 10.0f);
			    ImGui::InputInt("Number of particles", &params.n_of_particles);
                if (ImGui::Button("Start")) {
                    start = !start;
				    if (start) {
					    InitParticles(particles, &params);
                        spinZPlot.reserve(params.n_of_particles);
                        printf("Looking for ground state...\n");
				    }
                }
            ImGui::End();


			// Control the stages of the simulation and create live plots accordingly
            if (start) {
                float current_energy = getTotalEnergy(particles, &params);
                if (energyPlot.size() > 0 && abs(current_energy - energyPlot.back()) < 0.001f && !found_ground_state) {
                    printf("Found ground state! Removed precession damping.\n");
                    params.damping = 0.0005f;
                    params.dt_ps = 0.01f;
                    found_ground_state = true;
                }
                energyPlot.push_back(current_energy);
                ImGui::Begin("Energy");
                    ImGui::Text("Total energy: %.4f meV", current_energy);
                    ImGui::Text("Current damping: %f.2", params.damping);
                    ImGui::PlotLines("Energy v time", energyPlot.data(), energyPlot.size(),
                        0, "E(meV)", FLT_MAX, FLT_MAX, ImVec2(0, 150));
                ImGui::End();
                if (found_ground_state) {
                    ImGui::Begin("Clock");
                        ImGui::Text("%.4f ns", params.current_time / 1000.0f);
                    ImGui::End();
                    for (int i = 0; i < params.n_of_particles; i++) {
                        spinZPlot[i] = particles[i].spin.z;
                    }
                    ImGui::Begin("z components of spin");
                        ImGui::PlotLines("z components", spinZPlot.data(), params.n_of_particles, 0, NULL, FLT_MAX, FLT_MAX, ImVec2(0, 150));
                    ImGui::End();
                    ImGui::Begin("Create disturbance");
                        if (ImGui::Button("Magnetic pulse")) {
                            pulse_start_time = params.current_time;
                            params.ext_field_on = true;
                        }
                        ImGui::Text("Field status: %i", params.ext_field_on);
                    ImGui::End();
                    if (params.ext_field_on && (params.current_time - pulse_start_time > params.ext_field_pulse_lenght) ) {
                        params.ext_field_on = false;
                        printf("Added magnetic pulse of lenght %f.2!\n", params.ext_field_pulse_lenght);
                    }
                }
            }
            rlImGuiEnd();
        EndDrawing();
    }
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}



