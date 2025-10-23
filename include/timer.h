#ifndef TIMER_H
#define TIMER_H

#include <omp.h>  // OpenMP header

// Simple Timer with elapsed() method using OpenMP
class Timer {
public:
    Timer() : start_(omp_get_wtime()) {}

    // Reset the start time
    void reset() { start_ = omp_get_wtime(); }

    // Return elapsed time in seconds (double)
    double elapsed() const {
        return omp_get_wtime() - start_;
    }

private:
    double start_;
};

#endif // TIMER_H
