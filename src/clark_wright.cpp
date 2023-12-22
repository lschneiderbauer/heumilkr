#include <cpp11.hpp>
#include <iterator>
#include <vector>
#include "routing_state.h"

using namespace cpp11;

template <typename T>
list vl_to_list(const std::unordered_set<std::shared_ptr<std::unordered_set<T>>> &vv)
{
  cpp11::writable::list lst;

  for (auto &v : vv)
  {
    // convert un-ordered set to vector
    std::vector<T> v2{ std::make_move_iterator(std::begin(*v)),
                  std::make_move_iterator(std::end(*v)) };

    lst.push_back(as_sexp(v2));
  }

  return lst;
}


[[cpp11::register]]
list cpp_clark_wright(const std::vector<double> &demand,
                              const std::vector<double> &distances)
{

  RoutingState state(demand, distmat<double>(distances));

  printf("relinked-%d\n", state.relink_best());
  printf("===\n");
  printf("relinked-%d\n", state.relink_best());

  return vl_to_list(state.runs());
}
