#include <cpp11.hpp>
#include <iterator>
#include <vector>
#include "routing_state.h"

using namespace cpp11;

data_frame arrvec_to_dataframe(const col_types &cols)
{
  cpp11::writable::data_frame df({"site"_nm = as_sexp(std::get<0>(cols)),
                                  "run"_nm = as_sexp(std::get<1>(cols)),
                                  "order"_nm = as_sexp(std::get<2>(cols)),
                                  "vehicle"_nm = as_sexp(std::get<3>(cols)),
                                  "load"_nm = as_sexp(std::get<4>(cols)),
                                  "distance"_nm = as_sexp(std::get<5>(cols))});

  return df;
}

[[cpp11::register]]
data_frame cpp_clarke_wright(const std::vector<double> &demand,
                             const std::vector<double> &distances,
                             const std::vector<int> &n_res,
                             const std::vector<double> &capacities,
                             const std::vector<int> &restr_sites,
                             const std::vector<int> &restr_vehicles)
{
  std::vector<std::unordered_set<int>> restricted_vehicles(demand.size());
  for (unsigned int i = 0; i < restr_sites.size(); i++)
  {
    restricted_vehicles[restr_sites[i]].insert(restr_vehicles[i]);
  }

  routing_state state(demand, distmat<double>(distances), n_res,
                      capacities, restricted_vehicles);

  while (state.relink_best()) {};

  // After we have the final routes, we might still be able to assign
  // better vehicles for each route
  // (we might have released some high-priority vehicles on the way which
  // are now unused)
  while(state.opt_vehicles()) {};

  return arrvec_to_dataframe(state.runs_as_cols());
}

[[cpp11::register]]
data_frame cpp_clarke_wright_unr(const std::vector<double> &demand,
                                 const std::vector<double> &distances,
                                 const std::vector<int> &n_res,
                                 const std::vector<double> &capacities)
{
  std::vector<int> restr_sites;
  std::vector<int> restr_vehicles;

  return cpp_clarke_wright(demand, distances, n_res, capacities,
                    restr_sites, restr_vehicles);
}


[[cpp11::register]]
list cpp_clarke_wright_stepwise(const std::vector<double> &demand,
                                const std::vector<double> &distances,
                                const std::vector<int> &n_res,
                                const std::vector<double> &capacities,
                                const std::vector<int> &restr_sites,
                                const std::vector<int> &restr_vehicles)
{
  std::vector<std::unordered_set<int>> restricted_vehicles(demand.size());
  for (unsigned int i = 0; i < restr_sites.size(); i++)
  {
    restricted_vehicles[restr_sites[i]].insert(restr_vehicles[i]);
  }

  routing_state state(demand, distmat<double>(distances), n_res,
                      capacities, restricted_vehicles);

  cpp11::writable::list steps;
  steps.push_back(arrvec_to_dataframe(state.runs_as_cols()));

  while (state.relink_best())
  {
    steps.push_back(arrvec_to_dataframe(state.runs_as_cols()));
  };

  return steps;
}
