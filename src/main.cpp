#include <cstdio>
#include <vector>
#include <cstdlib>
#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "utils.h"
#include <math.h>
#include "DataLogger.h"


int main(void) {
    InitWindow(1800, 500, "1D chain spin wave simulation");
    rlImGuiSetup(true);

	// init variables
    Params params;
    bool start = false;
    bool found_ground_state = false;
    bool recording = false;
    float pulse_start_time = 0.0f;
    std::vector<Particle> particles;
    std::vector<float> energyPlot(1000);
	int energy_plot_counter = 0;
	std::vector<float> spinZPlot;
    float current_time = 0.0f;
    DataLogger* logger = nullptr;
    float rec_end_time = 0.0f;
	int steps_per_frame = 1;
	int rec_counter = 0;

	// init camera for animation
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 50.0f, 0.0f, -35.0f };
    camera.target = (Vector3){ 50.0f, 5.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    SetTargetFPS(30);
    Shader lightShader = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");

    while (!WindowShouldClose())
    {
            if (start) {
				for (int i = 0; i < steps_per_frame; i++) {
					// Run physics
                	MinimizeEnergy(particles, &params);

                    // Record and analyze data
                    if (rec_end_time != 0.0f) {
                        if (current_time < rec_end_time && rec_counter == 100) {
                            logger->listen(spinZPlot, &params);
							rec_counter = 0;
                        }
                        else if (current_time >= rec_end_time) {
                            logger->analyze(&params);
                            printf("Simulation finished! Thank you and goodbye.\n");
							rec_end_time = 0.0f;
							delete logger;
						}
						rec_counter++;
					}

					// Increment time
					current_time += params.dt_ps;
				}
			}
                // Draw the animation
			BeginDrawing();

                ClearBackground(DARKGRAY);
				if (start) {
	                BeginMode3D(camera);
	                BeginShaderMode(lightShader);
	                    for (int i = 0; i < params.n_of_particles; i++) {
	                        DrawArrow(&particles[i], &params);
	                    }
						DrawAxes(&params);
	                    if (params.ext_field_on) {DrawFieldVisual(&params);}
	                EndShaderMode();
	                EndMode3D();
				}


	            // Create the GUI
	            rlImGuiBegin();

		            ImGui::Begin("Settings");
	                ImGui::SliderFloat("Scale", &params.scale, 0.2f, 20.0f);
	                ImGui::SliderFloat("J1", &params.J1, -5.0f, 5.0f);
	                ImGui::SliderFloat("J2", &params.J2, -5.0f, 5.0f);
	                ImGui::SliderFloat("External field", &params.external_field, 0.0f, 10.0f);
	                ImGui::SliderFloat("Pulse lenght:", &params.ext_field_pulse_lenght, 0.1f, 5.0f);
	                ImGui::SliderFloat("Field radius", &params.external_field_radius, 5.0f, 10.0f);
	                ImGui::SliderFloat("Energy resolution", &params.energy_resolution, 0.001f, 0.005f);
				    ImGui::InputInt("Number of particles", &params.n_of_particles);
	                if (ImGui::Button("Start")) {
	                    start = !start;
					    if (start) {
						    InitParticles(particles, &params);
	                        spinZPlot.resize(params.n_of_particles, 0.0f);
	                        printf("Looking for ground state...\n");
					    }
	                }
		            ImGui::End();


				// CONTROL THE STAGES OF SIMULATION

	            if (start) {
	                // Energy plot
	                float current_energy = getTotalEnergy(particles, &params);
					if (current_time > 2.0f && !found_ground_state) {
						float sum = 0.0f;
						for (int i = 1; i <= 10; i++) {
							sum += fabsf(energyPlot[energy_plot_counter - 1] - energyPlot[energy_plot_counter - 2]);
						}
						float min_sum = (params.n_of_particles / 100000.0f) * 5;
						printf("Current: %f.2 -- Target: %f.2\n", sum, min_sum);
						if (sum < min_sum) {
		                    printf("Found ground state! Removed precession damping.\n");
			                params.damping = 0.0001f;
	    	                params.dt_ps = 0.001f;
							steps_per_frame = 200;
		                    found_ground_state = true;
							current_time = 0.0f;
						}
					}
					if (energy_plot_counter == 1000) {
						energy_plot_counter = 0;
					}
	                energyPlot[energy_plot_counter] = current_energy;
					energy_plot_counter++;
	                ImGui::Begin("Energy");
	                    ImGui::Text("Total energy: %.4f meV", current_energy);
	                    ImGui::Text("Current damping: %f.2", params.damping);
	                    ImGui::PlotLines("Energy v time", energyPlot.data(), energyPlot.size(),
	                        0, "E(meV)", FLT_MAX, FLT_MAX, ImVec2(0, 150));
	                ImGui::End();

	                // STAGE 2 (GROUND STATE FOUND)
	                if (found_ground_state) {
	                    // Clock
	                    ImGui::Begin("Clock");
	                        ImGui::Text("%.4f ns", current_time / 1000.0f);
	                    ImGui::End();

	                    // S_z plot
	                    for (int i = 0; i < params.n_of_particles; i++) {
	                        spinZPlot[i] = particles[i].spin.z;
	                    }
	                    ImGui::Begin("z components of spin");
	                        ImGui::PlotLines("z components", spinZPlot.data(), params.n_of_particles, 0, NULL, FLT_MAX, FLT_MAX, ImVec2(0, 150));
	                    ImGui::End();

	                    // Add external field
	                    ImGui::Begin("Create disturbance");
	                        if (ImGui::Button("Magnetic pulse")) {
	                            pulse_start_time = current_time;
	                            params.ext_field_on = true;
	                            printf("Adding magnetic pulse of lenght %f.2!\n", params.ext_field_pulse_lenght);
	                        }
	                        ImGui::Text("Field status: %s", params.ext_field_on ? "on" : "off");
	                    ImGui::End();
	                    if (params.ext_field_on && (current_time - pulse_start_time > params.ext_field_pulse_lenght) ) {
	                        printf("Done!\n");
	                        params.ext_field_on = false;
	                        float recording_time = (2 * M_PI * params.hbar) / params.energy_resolution;
							printf("Starting recording...\n");
							printf("Recording time: %f.2\n", recording_time);
	                        int size_hint = (int) (recording_time / params.dt_ps);
	                        rec_end_time = current_time + recording_time;
							printf("Recording from %f.2 to %f.2...\n", current_time, current_time + recording_time);
	                        logger = new DataLogger(size_hint);

	                    }



                    }

                }
            rlImGuiEnd();
        	EndDrawing();
    };
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}



