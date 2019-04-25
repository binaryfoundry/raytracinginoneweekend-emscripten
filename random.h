#pragma once

#ifdef _WIN32
using std::default_random_engine;
using std::uniform_real_distribution;

default_random_engine generator;
uniform_real_distribution<double> distribution(0.0, 1.0);

inline float drand48() { return distribution(generator); }
#endif
