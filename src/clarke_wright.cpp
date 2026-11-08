#include "clarke_wright.h"
#include <vector>
#include <functional>
#include "router.h"
#include "symmat.h"
#include "input_error.h"

tbls cpp_clarke_wright(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles,
    std::function<void(Router &)> callback)
{
  if (const auto error = validate_input(
          demand, distances, n_res, capacities, restr_sites, restr_vehicles))
  {
    throw InputException(*error);
  }

  std::vector<std::unordered_set<int>> restricted_vehicles(demand.size());
  for (unsigned int i = 0; i < restr_sites.size(); i++)
  {
    restricted_vehicles[restr_sites[i]].insert(restr_vehicles[i]);
  }

  auto fleet = std::make_shared<Fleet>(n_res, capacities, restricted_vehicles);
  Distmat distm(distances);

  // Demands can have any sign, and both signs are handled in a single pass:
  // positive = site is a sink and the origin is the source
  // negative = site is a source and the origin is the sink
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
  callback(router); // the last step must equal the final result

  return (router.runs_as_tbls());
}

#ifdef HEUMILKR_DEBUG_MAIN
// only for debug purposes, see Makefile.debug

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
    printf("Run: %d, Site: %d, Order: %d, Load after visit: %f\n",
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
