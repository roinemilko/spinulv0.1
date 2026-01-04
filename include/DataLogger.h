#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <fftw3.h>
#include <cstdlib>
#include <vector>
#include <string>

class DataLogger {
    private:
        std::vector<float> _data;
		std::vector<Particle>  start;
    public:
        DataLogger(int size_hint, std::vector<Particle>& initial_state);
        void listen(std::vector<float>& data, Params* params);
        void analyze(Params* params);
};

#endif