#include "fleet.h"
#include <limits.h>
#include <stdexcept>
#include <unordered_set>
#include "union_view.h"

int Fleet::capacity(const int vehicle) const
{
    return vehicle_caps[vehicle];
}

void Fleet::release_vehicle(const int vehicle)
{
    if (vehicle_avail[vehicle] < INT_MAX)
    {
        vehicle_avail[vehicle] += 1;
    }
}

void Fleet::reserve_vehicle(const int vehicle)
{
    if (vehicle == -1)
    {
        throw std::runtime_error("blah.");
    }

    if (vehicle_avail[vehicle] <= 0)
    {
        throw std::runtime_error("No available vehicles of this type to pop.");
    }

    if (vehicle_avail[vehicle] < INT_MAX)
    {
        vehicle_avail[vehicle] -= 1;
    }
}

template <typename ForwardIt>
bool Fleet::is_vehicle_restricted(const int vehicle, ForwardIt sites) const
{
    bool restricted = false;

    for (const auto site : sites)
    {
        restricted = restricted ||
                     (this->restricted_vehicles[site].find(vehicle) !=
                      this->restricted_vehicles[site].end());
    }

    return restricted;
}

template <typename ForwardIt>
int Fleet::find_fitting_vehicle(ForwardIt sites, const double max_load, bool max_load_soft) const
{
    for (size_t vehicle = 0; vehicle < vehicle_caps.size(); vehicle++)
    {
        if (vehicle_avail[vehicle] >= 1 &&
            max_load <= vehicle_caps[vehicle] &&
            !is_vehicle_restricted(vehicle, sites))
        {
            return vehicle;
        }
    }

    // max_load is only a soft restriction
    if (max_load_soft)
    {
        for (size_t vehicle = 0; vehicle < vehicle_caps.size(); vehicle++)
        {
            if (vehicle_avail[vehicle] >= 1 &&
                !is_vehicle_restricted(vehicle, sites))
            {
                return vehicle;
            }
        }
    }

    return -1;
}

template int Fleet::find_fitting_vehicle(union_view<int, std::unordered_set>, const double, bool) const;
template int Fleet::find_fitting_vehicle(std::unordered_set<int>, const double, bool) const;
