#include "fleet.h"
#include <limits.h>
#include <stdexcept>
#include <unordered_set>
#include "union_view.h"

int Fleet::capacity(const int vehicle) const
{
    return vehicle_caps[vehicle];
}

void Fleet::release_vehicle(const VehicleTypeID vehicle)
{
    if (vehicle_avail[vehicle] < INT_MAX)
    {
        vehicle_avail[vehicle] += 1;
    }
}

void Fleet::reserve_vehicle(const VehicleTypeID vehicle)
{
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
bool Fleet::is_vehicle_restricted(const VehicleTypeID vehicle, ForwardIt sites) const
{
    for (const auto site : sites)
    {
        if (this->restricted_vehicles[site].find(vehicle) != this->restricted_vehicles[site].end())
        {
            return true;
        }
    }

    return false;
}

template <typename ForwardIt>
std::optional<VehicleTypeID> Fleet::find_fitting_vehicle(ForwardIt sites, const double max_load, bool max_load_soft) const
{
    for (size_t vehicle = 0; vehicle < vehicle_caps.size(); vehicle++)
    {
        if (vehicle_avail[vehicle] >= 1 &&
            max_load <= vehicle_caps[vehicle] &&
            !is_vehicle_restricted(vehicle, sites))
        {
            return (VehicleTypeID)vehicle;
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
                return (VehicleTypeID)vehicle;
            }
        }
    }

    return std::nullopt;
}

template std::optional<VehicleTypeID> Fleet::find_fitting_vehicle(union_view<int, std::list>, const double, bool) const;
template std::optional<VehicleTypeID> Fleet::find_fitting_vehicle(std::list<int>, const double, bool) const;
