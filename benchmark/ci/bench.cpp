// Benchmark driver for the C++ core, used by .github/scripts/perf-compare.sh to
// catch performance regressions in CI. It depends on nothing but the core in
// src/, so it can be built against older versions of it, too.
//
// Usage: bench <scenario>
//
// Runs one scenario and prints a checksum of the solution. When run under
// cachegrind with --instr-at-start=no, only the solver call is measured.
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "clarke_wright.h"

#if __has_include(<valgrind/cachegrind.h>)
#include <valgrind/cachegrind.h>
#endif
#ifndef CACHEGRIND_START_INSTRUMENTATION
#define CACHEGRIND_START_INSTRUMENTATION
#define CACHEGRIND_STOP_INSTRUMENTATION
#endif

namespace
{
  // splitmix64: tiny and fully specified, so instances are identical for any
  // compiler or standard library
  struct Rng
  {
    std::uint64_t state;

    std::uint64_t next()
    {
      std::uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
      z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
      z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
      return z ^ (z >> 31);
    }

    // uniform in [lo, hi)
    double uniform(double lo, double hi)
    {
      return lo + (hi - lo) * double(next() >> 11) * 0x1.0p-53;
    }
  };

  struct Instance
  {
    std::vector<double> demand;
    std::vector<double> distances;
    std::vector<int> n_res;
    std::vector<double> capacities;
    std::vector<int> restr_sites;
    std::vector<int> restr_vehicles;
  };

  // sites spread uniformly around the depot at (0, 0), euclidean distances
  // as upper triangle row by row, see clarke_wright.h
  std::vector<double> random_distances(Rng &rng, int n_sites)
  {
    std::vector<double> x{0}, y{0};
    for (int i = 0; i < n_sites; i++)
    {
      x.push_back(rng.uniform(-10, 10));
      y.push_back(rng.uniform(-10, 10));
    }

    std::vector<double> distances;
    distances.reserve(std::size_t(n_sites + 1) * n_sites / 2);
    for (int i = 0; i <= n_sites; i++)
    {
      for (int j = i + 1; j <= n_sites; j++)
      {
        distances.push_back(std::hypot(x[i] - x[j], y[i] - y[j]));
      }
    }
    return distances;
  }

  // deliveries only, a single vehicle type without limit
  Instance uniform(int n_sites, std::uint64_t seed)
  {
    Rng rng{seed};
    Instance inst;
    for (int i = 0; i < n_sites; i++)
    {
      inst.demand.push_back(rng.uniform(5, 15));
    }
    inst.distances = random_distances(rng, n_sites);
    inst.n_res = {n_sites};
    inst.capacities = {33};
    return inst;
  }

  // deliveries and pickups (backhaul)
  Instance mixed(int n_sites, std::uint64_t seed)
  {
    Rng rng{seed};
    Instance inst;
    for (int i = 0; i < n_sites; i++)
    {
      const double amount = rng.uniform(5, 15);
      inst.demand.push_back(rng.uniform(0, 1) < 0.3 ? -amount : amount);
    }
    inst.distances = random_distances(rng, n_sites);
    inst.n_res = {n_sites};
    inst.capacities = {33};
    return inst;
  }

  // several vehicle types, some limited, and site restrictions
  Instance fleet(int n_sites, std::uint64_t seed)
  {
    Rng rng{seed};
    Instance inst;
    for (int i = 0; i < n_sites; i++)
    {
      inst.demand.push_back(rng.uniform(5, 15));
    }
    inst.distances = random_distances(rng, n_sites);
    inst.n_res = {n_sites, n_sites / 10, n_sites / 20};
    inst.capacities = {20, 33, 50};
    for (int i = 0; i < n_sites; i += 7)
    {
      inst.restr_sites.push_back(i);
      inst.restr_vehicles.push_back(2);
    }
    return inst;
  }

  bool make_instance(const std::string &scenario, Instance &inst)
  {
    if (scenario == "small_100")
      inst = uniform(100, 1);
    else if (scenario == "uniform_2000")
      inst = uniform(2000, 2);
    else if (scenario == "mixed_2000")
      inst = mixed(2000, 3);
    else if (scenario == "fleet_1000")
      inst = fleet(1000, 4);
    else
      return false;
    return true;
  }
} // namespace

int main(int argc, char **argv)
{
  Instance inst;
  if (argc != 2 || !make_instance(argv[1], inst))
  {
    std::fprintf(stderr,
                 "usage: bench <small_100|uniform_2000|mixed_2000|fleet_1000>\n");
    return 2;
  }

  CACHEGRIND_START_INSTRUMENTATION;
  const tbls result = cpp_clarke_wright(
      inst.demand, inst.distances, inst.n_res, inst.capacities,
      inst.restr_sites, inst.restr_vehicles);
  CACHEGRIND_STOP_INSTRUMENTATION;

  // the checksum shows whether two builds found the same solution
  const auto &runs = std::get<1>(result);
  double total_distance = 0;
  for (const double d : std::get<3>(runs))
  {
    total_distance += d;
  }
  std::printf("runs=%zu distance=%.6f\n", std::get<0>(runs).size(),
              total_distance);
  return 0;
}
