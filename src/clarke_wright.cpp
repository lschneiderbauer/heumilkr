#include <cpp11.hpp>
#include <iterator>
#include <vector>
#include "routing_state.h"

using namespace cpp11;

template <typename T1, typename T2, typename T3>
list triple_to_list(const std::vector<std::tuple<T1, T2, T3>> vec)
{
  cpp11::writable::list lst;

  std::vector<T1> v1(vec.size());
  std::vector<T2> v2(vec.size());
  std::vector<T3> v3(vec.size());

  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    int i = std::distance(vec.begin(), it);

    v1[i] = std::get<0>(*it);
    v2[i] = std::get<1>(*it);
    v3[i] = std::get<2>(*it);
  }

  lst.push_back(as_sexp(v1));
  lst.push_back(as_sexp(v2));
  lst.push_back(as_sexp(v3));

  return lst;
}

[[cpp11::register]] list cpp_clarke_wright(const std::vector<double> &demand,
                                          const std::vector<double> &distances,
                                          const std::vector<int> &n_res,
                                          const std::vector<double> &capacities)
{
  RoutingState state(demand, distmat<double>(distances), n_res, capacities);

  while (state.relink_best())
  {
    //printf("===\n");
  };

  return triple_to_list<int, int, int>(state.runs_as_cols());
}
