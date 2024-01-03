#include <cpp11.hpp>
#include <iterator>
#include <vector>
#include "routing_state.h"

using namespace cpp11;

data_frame arrvec_to_dataframe(const std::array<std::vector<int>, 4> &cols)
{
  cpp11::writable::data_frame df({"site"_nm = as_sexp(cols[0]),
                                  "run"_nm = as_sexp(cols[1]),
                                  "order"_nm = as_sexp(cols[2]),
                                  "vehicle"_nm = as_sexp(cols[3])});

  return df;
}

[[cpp11::register]] data_frame cpp_clarke_wright(const std::vector<double> &demand,
                                                 const std::vector<double> &distances,
                                                 const std::vector<int> &n_res,
                                                 const std::vector<double> &capacities)
{
  routing_state state(demand, distmat<double>(distances), n_res, capacities);

  while (state.relink_best())
  {
    // printf("===\n");
  };

  return arrvec_to_dataframe(state.runs_as_cols());
}

[[cpp11::register]] list cpp_clarke_wright_stepwise(const std::vector<double> &demand,
                                                    const std::vector<double> &distances,
                                                    const std::vector<int> &n_res,
                                                    const std::vector<double> &capacities)
{
  routing_state state(demand, distmat<double>(distances), n_res, capacities);

  cpp11::writable::list steps;
  steps.push_back(arrvec_to_dataframe(state.runs_as_cols()));

  while (state.relink_best())
  {
    steps.push_back(arrvec_to_dataframe(state.runs_as_cols()));
    ;
    // printf("===\n");
  };

  return steps;
}
