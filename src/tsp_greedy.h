#ifndef TSP_GREEDY
#define TSP_GREEDY

#include <vector>
#include <unordered_set>
#include "symmat.h"

template <typename Container>
std::tuple<Container, double> tsp_greedy(const Container &sites, const Distmat &distances);

#endif