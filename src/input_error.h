#ifndef INPUT_ERROR_H
#define INPUT_ERROR_H

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <vector>

// The ways in which the solver input can be invalid.
//
// The core only reports what is wrong and where. Each binding turns that into
// an error message in the terms of its own language (argument names, whether
// indices start at 0 or 1, ...), so that messages look familiar to its users.
//
// The meaning of the fields of `InputError` depends on the code. Locations are
// numbered as in the distances: 0 is the depot, and `i + 1` is site `i`.
enum class InputErrorCode
{
  // `index`: the site. `value`: its demand.
  non_finite_demand,
  // `index`: the number of distances, `other_index`: the expected number.
  wrong_distance_count,
  // `index`, `other_index`: the two locations. `value`: their distance.
  non_finite_distance,
  // There is not a single vehicle type.
  no_vehicle_types,
  // `index`: the number of capacities, `other_index`: the number of counts.
  vehicle_length_mismatch,
  // `index`: the vehicle type. `value`: its count.
  negative_vehicle_count,
  // `index`: the vehicle type. `value`: its capacity, which is not positive
  // or not finite.
  invalid_vehicle_capacity,
  // `index`: the number of restricted vehicles, `other_index`: the number of
  // restricted sites.
  restriction_length_mismatch,
  // `index`: the restriction. `value`: the site it refers to.
  unknown_restriction_site,
  // `index`: the restriction. `value`: the vehicle type it refers to.
  unknown_restriction_vehicle,
  // The solver starts with one run per site, and ran out of vehicles for
  // those. `index`: the site it ran out at.
  insufficient_vehicles,
};

struct InputError
{
  InputErrorCode code;
  std::size_t index = 0;
  std::size_t other_index = 0;
  double value = 0;
};

// Thrown by the solver on invalid input.
class InputException : public std::invalid_argument
{
public:
  explicit InputException(const InputError &error);
  const InputError &error() const { return error_; }

private:
  InputError error_;
};

// A stable snake_case name for each code, e.g. "non_finite_demand", which the
// bindings use to look up their messages.
const char *input_error_name(InputErrorCode code);

// All codes, so that the bindings can test that each one has a message.
std::vector<InputErrorCode> all_input_error_codes();

// Checks the solver input, see cpp_clarke_wright() for its meaning. Returns
// the first problem found, in the order of the arguments.
std::optional<InputError> validate_input(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles);

#endif // INPUT_ERROR_H
