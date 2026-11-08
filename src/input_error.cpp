#include "input_error.h"
#include <array>
#include <cmath>
#include <sstream>
#include <string>

namespace
{
  constexpr std::array<InputErrorCode, 11> codes = {
      InputErrorCode::non_finite_demand,
      InputErrorCode::wrong_distance_count,
      InputErrorCode::non_finite_distance,
      InputErrorCode::no_vehicle_types,
      InputErrorCode::vehicle_length_mismatch,
      InputErrorCode::negative_vehicle_count,
      InputErrorCode::invalid_vehicle_capacity,
      InputErrorCode::restriction_length_mismatch,
      InputErrorCode::unknown_restriction_site,
      InputErrorCode::unknown_restriction_vehicle,
      InputErrorCode::insufficient_vehicles,
  };

  // fails to compile when a code is added to the enum, but not to `codes`
  static_assert(codes.size() ==
                static_cast<std::size_t>(InputErrorCode::insufficient_vehicles) + 1);

  // A plain English message, for callers that do not bring their own, and so
  // that what() says something useful. Indices start at 0.
  std::string describe(const InputError &e)
  {
    std::ostringstream msg;

    switch (e.code)
    {
    case InputErrorCode::non_finite_demand:
      msg << "The demand of site " << e.index << " is not finite: " << e.value;
      break;
    case InputErrorCode::wrong_distance_count:
      msg << "Expected " << e.other_index << " distances, got " << e.index;
      break;
    case InputErrorCode::non_finite_distance:
      msg << "The distance between locations " << e.index << " and "
          << e.other_index << " is not finite: " << e.value;
      break;
    case InputErrorCode::no_vehicle_types:
      msg << "At least one vehicle type is required";
      break;
    case InputErrorCode::vehicle_length_mismatch:
      msg << "Got " << e.index << " vehicle capacities, but "
          << e.other_index << " vehicle counts";
      break;
    case InputErrorCode::negative_vehicle_count:
      msg << "The count of vehicle type " << e.index
          << " is negative: " << e.value;
      break;
    case InputErrorCode::invalid_vehicle_capacity:
      msg << "The capacity of vehicle type " << e.index
          << " is not positive and finite: " << e.value;
      break;
    case InputErrorCode::restriction_length_mismatch:
      msg << "Got " << e.index << " restricted vehicles, but "
          << e.other_index << " restricted sites";
      break;
    case InputErrorCode::unknown_restriction_site:
      msg << "Restriction " << e.index << " refers to unknown site " << e.value;
      break;
    case InputErrorCode::unknown_restriction_vehicle:
      msg << "Restriction " << e.index
          << " refers to unknown vehicle type " << e.value;
      break;
    case InputErrorCode::insufficient_vehicles:
      msg << "Not enough vehicles available to start with one run per site,"
          << " ran out at site " << e.index;
      break;
    }

    return msg.str();
  }
}

InputException::InputException(const InputError &error)
    : std::invalid_argument(describe(error)), error_(error) {}

const char *input_error_name(InputErrorCode code)
{
  switch (code)
  {
  case InputErrorCode::non_finite_demand:
    return "non_finite_demand";
  case InputErrorCode::wrong_distance_count:
    return "wrong_distance_count";
  case InputErrorCode::non_finite_distance:
    return "non_finite_distance";
  case InputErrorCode::no_vehicle_types:
    return "no_vehicle_types";
  case InputErrorCode::vehicle_length_mismatch:
    return "vehicle_length_mismatch";
  case InputErrorCode::negative_vehicle_count:
    return "negative_vehicle_count";
  case InputErrorCode::invalid_vehicle_capacity:
    return "invalid_vehicle_capacity";
  case InputErrorCode::restriction_length_mismatch:
    return "restriction_length_mismatch";
  case InputErrorCode::unknown_restriction_site:
    return "unknown_restriction_site";
  case InputErrorCode::unknown_restriction_vehicle:
    return "unknown_restriction_vehicle";
  case InputErrorCode::insufficient_vehicles:
    return "insufficient_vehicles";
  }

  return "unknown";
}

std::vector<InputErrorCode> all_input_error_codes()
{
  return std::vector<InputErrorCode>(codes.begin(), codes.end());
}

std::optional<InputError> validate_input(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles)
{
  const std::size_t n_sites = demand.size();
  const std::size_t n_vehicles = capacities.size();

  for (std::size_t i = 0; i < n_sites; i++)
  {
    if (!std::isfinite(demand[i]))
    {
      return InputError{InputErrorCode::non_finite_demand, i, 0, demand[i]};
    }
  }

  const std::size_t n_locations = n_sites + 1;
  const std::size_t n_distances = n_locations * n_sites / 2;
  if (distances.size() != n_distances)
  {
    return InputError{
        InputErrorCode::wrong_distance_count, distances.size(), n_distances};
  }

  // upper triangle row by row, see cpp_clarke_wright()
  std::size_t k = 0;
  for (std::size_t i = 0; i < n_locations; i++)
  {
    for (std::size_t j = i + 1; j < n_locations; j++, k++)
    {
      if (!std::isfinite(distances[k]))
      {
        return InputError{InputErrorCode::non_finite_distance, i, j, distances[k]};
      }
    }
  }

  if (n_vehicles == 0)
  {
    return InputError{InputErrorCode::no_vehicle_types};
  }

  if (n_res.size() != n_vehicles)
  {
    return InputError{
        InputErrorCode::vehicle_length_mismatch, n_vehicles, n_res.size()};
  }

  for (std::size_t i = 0; i < n_vehicles; i++)
  {
    if (n_res[i] < 0)
    {
      return InputError{
          InputErrorCode::negative_vehicle_count, i, 0, double(n_res[i])};
    }

    if (!(capacities[i] > 0 && std::isfinite(capacities[i])))
    {
      return InputError{
          InputErrorCode::invalid_vehicle_capacity, i, 0, capacities[i]};
    }
  }

  if (restr_vehicles.size() != restr_sites.size())
  {
    return InputError{
        InputErrorCode::restriction_length_mismatch,
        restr_vehicles.size(),
        restr_sites.size()};
  }

  for (std::size_t i = 0; i < restr_sites.size(); i++)
  {
    const int site = restr_sites[i];
    if (site < 0 || std::size_t(site) >= n_sites)
    {
      return InputError{
          InputErrorCode::unknown_restriction_site, i, 0, double(site)};
    }

    const int vehicle = restr_vehicles[i];
    if (vehicle < 0 || std::size_t(vehicle) >= n_vehicles)
    {
      return InputError{
          InputErrorCode::unknown_restriction_vehicle, i, 0, double(vehicle)};
    }
  }

  return std::nullopt;
}
