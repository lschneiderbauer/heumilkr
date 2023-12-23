#include <vector>
#include <unordered_set>
#include "distmat.h"

std::vector<int> tsp_greedy(const std::unordered_set<int> sites,
                            const distmat<double> &distances);
