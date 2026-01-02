#include <vector>
#include <memory>
#include <unordered_set>

#ifndef RUN_H
#define RUN_H

// a run consists of an _unordered_ collection of site
class run
{
  public:
    std::unordered_set<int> sites;
    double max_load;
    int vehicle;

    run(int site, double max_load) // initialize a run with a single site
      : sites(std::unordered_set<int>{site}),
        max_load(max_load),
        vehicle(-1) { };
    run(int site, double max_load, int vehicle)
      : sites(std::unordered_set<int>{site}),
        max_load(max_load),
        vehicle(vehicle) { };
    void combine(run &other_run, int new_vehicle);
    
  private:
};

#endif