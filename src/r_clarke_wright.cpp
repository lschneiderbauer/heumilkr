#include <cpp11.hpp>
#include <iterator>
#include <vector>
#include "runmanager.h"

using namespace cpp11;

cpp11::writable::data_frame rbind_df(const cpp11::writable::data_frame& a, const cpp11::writable::data_frame& b) {
  cpp11::function rbind = cpp11::package("base")["rbind"];
  return (cpp11::writable::data_frame)rbind(a, b);
}

cpp11::writable::data_frame arrvec_to_dataframe(const col_types &cols)
{
  cpp11::writable::data_frame df({"site"_nm = as_sexp(std::get<0>(cols)),
                                  "run"_nm = as_sexp(std::get<1>(cols)),
                                  "order"_nm = as_sexp(std::get<2>(cols)),
                                  "vehicle"_nm = as_sexp(std::get<3>(cols)),
                                  "load"_nm = as_sexp(std::get<4>(cols)),
                                  "distance"_nm = as_sexp(std::get<5>(cols))});

  return df;
}


col_types cpp_clarke_wright(const std::vector<double> &demand,
                          const std::vector<double> &distances,
                          const std::vector<int> &n_res,
                          const std::vector<double> &capacities,
                          const std::vector<int> &restr_sites,
                          const std::vector<int> &restr_vehicles);

[[cpp11::register]]
cpp11::writable::data_frame r_cpp_clarke_wright(const std::vector<double> &demand,
                             const std::vector<double> &distances,
                             const std::vector<int> &n_res,
                             const std::vector<double> &capacities,
                             const std::vector<int> &restr_sites,
                             const std::vector<int> &restr_vehicles)
{
  return
    arrvec_to_dataframe(
      cpp_clarke_wright(
        demand,
        distances,
        n_res,
        capacities,
        restr_sites,
        restr_vehicles
      )
    );

  // RunManager runm_comb(
  //   runm_pos,
  //   runm_neg,
  //   ind_pos,
  //   ind_neg
  // );
 
  //TODO combine those two solutions
  // return rbind_df(
  //   arrvec_to_dataframe(pos_state.runs_as_cols()),
  //   arrvec_to_dataframe(neg_state.runs_as_cols())
  // );
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
  Fleet fleet(n_res, capacities, restricted_vehicles);
  RunManager runm(demand, std::make_unique<distmat<double>>(distances),
                          std::make_shared<Fleet>(fleet));

  cpp11::writable::list steps;
  steps.push_back(arrvec_to_dataframe(runm.runs_as_cols()));

  while (runm.relink_best())
  {
    steps.push_back(arrvec_to_dataframe(runm.runs_as_cols()));
  };

  return steps;
}

