#include <cpp11.hpp>
#include <iterator>
#include <vector>
#include "routing_state.h"

using namespace cpp11;

template <typename T>
list vl_to_list(const std::vector<std::unordered_set<T>> &vv)
{
  cpp11::writable::list lst;

  for (auto &v : vv)
  {
    lst.push_back(as_sexp(v));
  }

  return (lst);
}


[[cpp11::register]]
list clark_wright(const std::vector<int> &demand,
                              const std::vector<double> &distances)
{

  RoutingState state(demand, symmat<double>(distances, demand.size() + 1));

  printf("%d\n", state.relink_best());
  printf("%d\n", state.relink_best());

  return (vl_to_list(state.runs()));
}
