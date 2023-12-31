#include <cpp11.hpp>
#include <iterator>
#include <vector>
#include "routing_state.h"

using namespace cpp11;

template <typename T1, typename T2>
list pair_to_list(const std::vector<std::pair<T1, T2>> vec)
{
  cpp11::writable::list lst;

  std::vector<T1> v1(vec.size());
  std::vector<T2> v2(vec.size());

  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    int i = std::distance(vec.begin(), it);

    v1[i] = (*it).first;
    v2[i] = (*it).second;
  }

  lst.push_back(as_sexp(v1));
  lst.push_back(as_sexp(v2));

  return lst;
}

[[cpp11::register]] list cpp_clark_wright(const std::vector<double> &demand,
                                          const std::vector<double> &distances,
                                          const std::vector<int> &n_res,
                                          const std::vector<double> &capacities)
{
  RoutingState state(demand, distmat<double>(distances), n_res, capacities);

  while (state.relink_best())
  {
    printf("===\n");
  };

  return pair_to_list<int, int>(state.runs_as_cols());
}
