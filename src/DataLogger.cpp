#include "utils.h"
#include "DataLogger.h"
#include <fftw3.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>
#include <math.h>

DataLogger::DataLogger(int size_hint, std::vector<Particle>& initial_state) {
    _data.reserve(size_hint);
	start = initial_state;
}

void DataLogger::listen(std::vector<float>& data, Params* params) {
    for (int i = 0; i < params->n_of_particles; i++) {
        _data.push_back(data[i] - start[i].spin.z);
    }

}

void DataLogger::analyze(Params* params) {
    int N = params->n_of_particles;
    int T = _data.size() / N;

	printf("Planning the discrete fourier transform...\n");
    // Plan the DFT
	int n_of_bins = N / 2 + 1;
    int complex_out_size = T * n_of_bins;
    fftwf_complex* out = fftwf_alloc_complex(complex_out_size);
	float* in = fftwf_alloc_real(N * T);
	if (!out || !in) {
		perror("Failed to allocate space for the discrete fourier transform\n");
		return;
	}
    fftwf_plan plan = fftwf_plan_dft_r2c_2d(T, N, in, out, FFTW_ESTIMATE);
	printf("T = %d\n", T);
	if (plan == NULL) {
		perror("Failed to create plan\n");
		free(out);
		free(in);
		return;
	}

    // Initialize the input
	printf("Plam finished! Initializing the input... \n");

    for (int i = 0; i < (T * N); i++) {
        in[i] = _data[i];
    }

	// Tukey window
	#pragma omp parallel for
    for (int t = 0; t < T; t++) {
        float window = 1.0f;
        if (t < params->window_param * T / 2.0f) {
            window = 0.5f * (1.0f + cosf(M_PI * (2.0f * t / (params->window_param * T) - 1.0f)));
        }
        else if (t > T * (1.0f - params->window_param / 2.0f)) {
            float dist = T - 1 - t;
            window = 0.5f * (1.0f + cosf(M_PI * (2.0f * dist / (params->window_param * T) - 1.0f)));
        }

        for (int p = 0; p < N; p++) {
            in[t * N + p] *= window;
        }
    }

	printf("Executing fourier transorm...\n");
	// execute DFT
    fftwf_execute(plan);

	printf("Saving results...\n");
    // Print results to a csv file
    FILE* filePtr = fopen("result_big3.csv", "w");
    if (filePtr == nullptr) {
        perror("Couldn't create result.csv");
    }
	float normalize = 1.0f / float(T*N);
    for (int t = 0; t < T; t++) {
		for (int k = 0; k < (N / 2 + 1); k++) {
			int idx = t * n_of_bins + k;
 	    	float magnitude = sqrt( (out[idx][0] * out[idx][0]) + (out[idx][1] * out[idx][1]) ) * 2 * normalize;
			fprintf(filePtr, "%f%s", magnitude, (k == n_of_bins - 1) ? "" : ",");
		}
		fprintf(filePtr, "\n");
    }

    // Cleanup
    fclose(filePtr);
    fftwf_destroy_plan(plan);
    fftwf_free(out);
	fftwf_free(in);
}