#ifndef FLEET_H
#define FLEET_H

#include <vector>
#include <unordered_set>

class Fleet
{
public:
    Fleet(std::vector<int> &vehicle_avail, const std::vector<double> &vehicle_caps,
          const std::vector<std::unordered_set<int>> &restricted_vehicles)
        : vehicle_avail(vehicle_avail),
          vehicle_caps(vehicle_caps),
          restricted_vehicles(restricted_vehicles) {};
    void reserve_vehicle(const int vehicle);
    void release_vehicle(const int vehicle);

    template<typename ForwardIt>
    int find_fitting_vehicle(ForwardIt sites, const double max_load,
                            bool max_load_soft) const;
  
    int capacity(const int vehicle) const;

private:
  // Number and capacity of particular vehicle types
  std::vector<int> vehicle_avail;
  std::vector<double> vehicle_caps;

  // Vehicle restrictions per site
  std::vector<std::unordered_set<int>> restricted_vehicles;

  template<typename ForwardIt>
  bool is_vehicle_restricted(int vehicle, ForwardIt sites) const;
};

#endif