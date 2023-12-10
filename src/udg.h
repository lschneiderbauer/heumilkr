#include <unordered_set>
#include <vector>

#ifndef UDG_H
#define UDG_H

// undirected graph
class udg
{
public:
  udg(int n_vertices = 0);
  void add_edge(const int a, const int b);
  void remove_origin_edge(const int a);
  bool links_to_origin(const int a) const;
  bool links_only_to_origin(const int a) const;
  bool edges_share_cycle(const int a, const int b) const;
  std::vector<std::unordered_set<int>> cycles() const;

private:
  std::vector<std::unordered_set<int>> adj;
  // a vector of cycles: each vertex has a reference to
  // the cycle it belongs to (which in turn has all the other references)
  std::vector<std::unordered_set<int>> cycs;

};

#endif
