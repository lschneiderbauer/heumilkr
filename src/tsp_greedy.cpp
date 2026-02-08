#include <vector>
#include <unordered_set>
#include <limits>
#include <algorithm>
#include <list>
#include "tsp_greedy.h"
#include "site.h"

template <typename Container>
std::tuple<Container, double> tsp_greedy(const Container &sites, const Distmat &distances)
{
  Container run;
  // run.reserve(sites.size());

  double total_dist = 0;

  int ref_site = -1;
  int next_site = ref_site;

  std::unordered_set<Site> not_visited;
  for (const auto site : sites)
  {
    not_visited.insert(site);
  }

  int cont = true;
  do
  {
    double min_dist = std::numeric_limits<double>::max();
    // get the nearest site from origin
    for (const auto site : not_visited)
    {
      double dist = distances.get(ref_site + 1, site + 1);

      if (dist < min_dist)
      {
        min_dist = dist;
        next_site = site;
      }
    }

    if (next_site != ref_site)
    {
      run.push_back(next_site);
      not_visited.erase(next_site);
      ref_site = next_site;
      total_dist += min_dist;
    }
    else
    {
      cont = false;
    }

  } while (cont);

  total_dist += distances.get(0, next_site + 1);

  return std::make_tuple(run, total_dist);
}

template std::tuple<std::list<Site>, double> tsp_greedy(const std::list<Site> &sites, const Distmat &distances);