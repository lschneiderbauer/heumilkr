#include <vector>
#include <unordered_set>
#include <limits>
#include <algorithm>
#include "distmat.h"

std::vector<int> tsp_greedy(const std::unordered_set<int> sites,
                            const distmat<double> &distances)
{
  std::vector<int> run;
  run.reserve(sites.size());

  int ref_site = -1;
  int next_site = ref_site;

  int cont = true;
  do
  {
    double min_dist = std::numeric_limits<double>::max();
    // get the nearest site from origin
    for (auto &site : sites)
    {
      // if it's not already in the list check the distance
      if (std::find(run.begin(), run.end(), site) == run.end())
      {
        double dist = distances.get(ref_site + 1, site + 1);

        if (dist < min_dist)
        {
          min_dist = dist;
          next_site = site;
        }
      }
    }

    if (next_site != ref_site)
    {
      run.push_back(next_site);
      ref_site = next_site;
    }
    else
    {
      cont = false;
    }

  } while (cont);

  return run;
}
