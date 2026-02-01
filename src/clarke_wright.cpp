#include <iterator>
#include <vector>
#include "runmanager.h"
#include <cassert>

// selects demand with a given sign, and always returns positive values
std::tuple<std::vector<int>, std::vector<double>, distmat<double>>
select_demand(const std::vector<double> &demand, const distmat<double> &distm, int sign)
{
  std::vector<int> new_ind;
  std::vector<double> new_demand;
  distmat<double> new_distm;

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

  return std::tuple<std::vector<int>, std::vector<double>, distmat<double>>(new_ind, new_demand, new_distm);
}

tbls cpp_clarke_wright(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles,
    std::function<void(RunManager &)> callback = [](RunManager &) {})
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
  distmat<double> distm(distances);

  // we can have positive and negative demands
  // positive = sites are sinks and origin is source
  // negative = sites are sources and origin is sink
  //
  // Strategy:
  // Consider and solve positive and negative demands separately (we can use the same solver)
  // Combine those solutions afterwards

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

  /* Positive */

  std::vector<int> ind_pos;
  std::vector<double> demand_pos;
  distmat<double> distances_pos;

  if (have_pos)
  {
    std::tie(ind_pos, demand_pos, distances_pos) =
        select_demand(demand, distm, 1);
  }

  RunManager runm_pos(
      demand_pos,
      std::make_unique<distmat<double>>(distances_pos),
      fleet);

  if (have_pos)
  {
    callback(runm_pos);
    while (runm_pos.relink_best())
    {
      callback(runm_pos);
    };
    while (runm_pos.opt_vehicles())
    {
    };

    if (!have_neg)
    {
      return (runm_pos.runs_as_tbls(demand_pos));
    }
  }

  /* Negative */

  std::vector<int> ind_neg;
  std::vector<double> demand_neg;
  distmat<double> distances_neg;

  if (have_neg)
  {
    std::tie(ind_neg, demand_neg, distances_neg) =
        select_demand(demand, distm, -1);
  }

  RunManager runm_neg(demand_neg,
                      std::make_unique<distmat<double>>(distances_neg),
                      fleet);

  if (have_neg)
  {
    callback(runm_neg);
    while (runm_neg.relink_best())
    {
      callback(runm_neg);
    };

    while (runm_neg.opt_vehicles())
    {
    };

    if (!have_pos)
    {
      for (auto &dmnd : demand_neg)
      {
        dmnd = -dmnd;
      }
      return (runm_neg.runs_as_tbls(demand_neg));
    }
  }

  // if we have both, combine then and optimize again
  RunManager runm_all(runm_pos, runm_neg, distm, ind_pos, ind_neg);

  callback(runm_all);
  while (runm_all.relink_best([](double l1, double l2)
                              { return (std::max(l1, l2)); }))
  {
    callback(runm_all);
  };
  while (runm_all.opt_vehicles())
  {
  };

  return (runm_all.runs_as_tbls(demand));
}

#ifndef NDEBUG
// only for debug purposes
int main()
{
  tbls cols =
      cpp_clarke_wright(
          std::vector<double>{-3, -2, 1, 2},
          std::vector<double>{5, 10, 5, 4, 5, 2, 12, 13, 10, 3},
          std::vector<int>{100},
          std::vector<double>{5},
          std::vector<int>{},
          std::vector<int>{});

  tbl_run_site tr = std::get<2>(cols);

  for (size_t i = 0; i < std::get<0>(tr).size(); i++)
  {
    printf("Run: %d, Site: %d, Order: %d, Departing Load: %f\n",
           std::get<0>(tr)[i],
           std::get<1>(tr)[i],
           std::get<2>(tr)[i],
           std::get<3>(tr)[i]);
  }

  return 0;
}
#endif