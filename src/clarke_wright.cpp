#include <vector>
#include <functional>
#include "router.h"
#include "symmat.h"
#include <cassert>

// selects demand with a given sign, and always returns positive values
std::tuple<std::vector<int>, std::vector<double>, Distmat>
select_demand(const std::vector<double> &demand, const Distmat &distm, int sign)
{
  std::vector<int> new_ind;
  std::vector<double> new_demand;
  Distmat new_distm;

  new_ind.reserve(demand.size());
  new_demand.reserve(demand.size());
  for (size_t i = 0; i < demand.size(); i++)
  {
    if (demand[i] * sign > 0)
    {
      new_ind.push_back(i);
      new_demand.push_back(sign * demand[i]);
    }
  }

  if (new_ind.size() == demand.size())
  {
    new_distm = distm;
  }
  else
  {
    std::vector<int> new_ind_dist(new_ind.size() + 1);
    new_ind_dist[0] = 0;
    for (size_t i = 1; i < new_ind_dist.size(); i++)
    {
      new_ind_dist[i] = new_ind[i - 1] + 1;
    }
    new_distm = distm.sub(new_ind_dist);
  }

  return std::tuple<std::vector<int>, std::vector<double>, Distmat>(new_ind, new_demand, new_distm);
}

tbls cpp_clarke_wright(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles,
    std::function<void(Router &)> callback = [](Router &) {})
{
  // check that all inputs have the correct size
  assert(distances.size() == (demand.size() + 1) * (demand.size()) / 2);
  assert(capacities.size() == n_res.size());

  std::vector<std::unordered_set<int>> restricted_vehicles(demand.size());
  for (unsigned int i = 0; i < restr_sites.size(); i++)
  {
    restricted_vehicles[restr_sites[i]].insert(restr_vehicles[i]);
  }

  auto fleet = std::make_shared<Fleet>(n_res, capacities, restricted_vehicles);
  Distmat distm(distances);

  // we can have positive and negative demands
  // positive = sites are sinks and origin is source
  // negative = sites are sources and origin is sink
  //
  // Strategy:
  // Consider and solve positive and negative demands separately (we can use the same solver)
  // Combine those solutions afterwards
  /*
    bool have_pos = false;
    bool have_neg = false;

    for (size_t i = 0; i < demand.size(); i++)
    {
      if (demand[i] > 0)
      {
        have_pos = true;
      }
      if (demand[i] < 0)
      {
        have_neg = true;
      }
      if (have_pos && have_neg)
      {
        break;
      }
    }
  */
  /* Positive */
  /*
    std::vector<int> ind_pos;
    std::vector<double> demand_pos;
    distmat<double> distances_pos;

    if (have_pos)
    {
      std::tie(ind_pos, demand_pos, distances_pos) =
          select_demand(demand, distm, 1);
    }
  */

  // if we have both, combine then and optimize again
  Router router(
    std::make_shared<std::vector<double>>(demand),
    std::make_unique<Distmat>(distm), fleet);

  callback(router);
  while (router.relink_best())
  {
    callback(router);
  };
  while (router.optimize_vehicles())
  {
  };
  router.optimize_runs_order();

  return (router.runs_as_tbls());
}

#ifndef NDEBUG
// only for debug purposes

#include <stdio.h>

int main()
{
  tbls cols =
      cpp_clarke_wright(
          std::vector<double>{14.14, 14.37, 7.86},
          std::vector<double>{8.128214, 7.837354, 3.162988, 12.616152, 6.427298, 10.7306495},
          std::vector<int>{2, 100},
          std::vector<double>{33, 44},
          std::vector<int>{},
          std::vector<int>{});

  tbl_run_site trs = std::get<2>(cols);
  tbl_run tr = std::get<1>(cols);

  for (size_t i = 0; i < std::get<0>(trs).size(); i++)
  {
    printf("Run: %d, Site: %d, Order: %d, Departing Load: %f\n",
           std::get<0>(trs)[i],
           std::get<1>(trs)[i],
           std::get<2>(trs)[i],
           std::get<3>(trs)[i]);
  }

  for (size_t i = 0; i < std::get<0>(tr).size(); i++)
  {
    printf("Run: %d, Vehicle: %d, Max Load: %f, Distance: %f\n",
           std::get<0>(tr)[i],
           std::get<1>(tr)[i],
           std::get<2>(tr)[i],
           std::get<3>(tr)[i]);
  }

  return 0;
}
#endif


/*
> demand
[1] 14.148060 14.370754  7.861395
> pos
     pos_x     pos_y
1 0.000000  0.000000
2 6.608953  4.731766
3 2.834910 -7.306668
4 0.381919  3.139846
*/
