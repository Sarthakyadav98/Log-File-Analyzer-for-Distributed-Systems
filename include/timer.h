#ifndef TIMER_H
#define TIMER_H

#include <chrono>

// Simple Timer with elapsed() method
class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;

    Timer() : start_(Clock::now()) {}
    void reset() { start_ = Clock::now(); }

    // Return elapsed time in seconds (double)
    double elapsed() const {
        auto end = Clock::now();
        std::chrono::duration<double> diff = end - start_;
        return diff.count();
    }

private:
    Clock::time_point start_;
};

#endif // TIMER_H
