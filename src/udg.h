#include <unordered_set>
#include <vector>
#include <memory>

#ifndef UDG_H
#define UDG_H

// undirected graph
class udg
{
public:
  udg(int n_vertices = 0);
  void add_edge(const int a, const int b);
  void relink_edge(const int a, const int b);
  bool links_to_origin(const int a) const;
  bool edges_share_cycle(const int a, const int b) const;
  std::unordered_set<int> sites_in_cycle(const int a) const;
  std::vector<std::shared_ptr<std::unordered_set<int>>> get_cycs() const;

private:
  std::vector<std::unordered_set<int>> adj;
  // a vector of cycles: each vertex has a reference to
  // the cycle it belongs to (which in turn has all the other references)
  std::vector<std::shared_ptr<std::unordered_set<int>>> cycs;
};

#endif
