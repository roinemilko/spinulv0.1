#include "utils.h"
#include "DataLogger.h"
#include <fftw3.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>
#include <math.h>

DataLogger::DataLogger(int size_hint) {
    _data.reserve(size_hint);
}

void DataLogger::listen(std::vector<float>& data, Params* params) {
    for (int i = 0; i < params->n_of_particles; i++) {
        _data.push_back(data[i]);
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
	// (hanning window to reduce leakage)
	printf("Plam finished! Initializing the input... \n");
	#pragma omp parallel for
    for (int t = 0; t < T; t++) {
        float hanning = 0.5f * (1.0f - cosf( (2.0f * M_PI * t)/(float)(T-1) ) );
        for (int p = 0; p < N; p++) {
            in[t * N + p] = _data[t * N + p] * hanning;
        }
    }

	printf("Executing fourier transorm...\n");
	// execute DFT
    fftwf_execute(plan);

	printf("Saving results...\n");
    // Print results to a csv file
    FILE* filePtr = fopen("result.csv", "w");
    if (filePtr == nullptr) {
        perror("Couldn't create result.csv");
    }
    for (int t = 0; t < T; t++) {
		for (int k = 0; k < (N / 2 + 1); k++) {
			int idx = t * n_of_bins + k;
 	    	float magnitude = sqrt( (out[idx][0] * out[idx][0]) + (out[idx][1] * out[idx][1]) );
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