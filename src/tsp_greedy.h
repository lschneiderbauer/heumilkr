#ifndef TSP_GREEDY
#define TSP_GREEDY

#include "symmat.h"

template <typename Container>
std::tuple<Container, double> tsp_greedy(const Container &sites, const Distmat &distances);

#endif
