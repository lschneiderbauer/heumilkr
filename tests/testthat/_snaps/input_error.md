# Errors of the C++ core are reported in R terms

    Code
      clarke_wright(c(1, NA, 3), distances, unlimited)
    Condition
      Error in `clarke_wright()`:
      ! `demand` must not contain missing or infinite values.
      x `demand[2]` is NA.
    Code
      clarke_wright(c(1, 2, Inf), distances, unlimited)
    Condition
      Error in `clarke_wright()`:
      ! `demand` must not contain missing or infinite values.
      x `demand[3]` is Inf.
    Code
      clarke_wright(1:3, as.dist(missing_distance), unlimited)
    Condition
      Error in `clarke_wright()`:
      ! `distances` must not contain missing or infinite values.
      x The distance between locations "2" and "4" is NA.
    Code
      clarke_wright(1:3, distances, data.frame(n = integer(), caps = numeric()))
    Condition
      Error in `clarke_wright()`:
      ! `vehicles` must have at least one row.
    Code
      clarke_wright(1:3, distances, data.frame(n = c(NA, -2L), caps = 6))
    Condition
      Error in `clarke_wright()`:
      ! `vehicles$n` must not be negative.
      x `vehicles$n[2]` is -2.
      i Use `NA` for an unlimited number of vehicles.
    Code
      clarke_wright(1:3, distances, data.frame(n = NA, caps = c(6, 0)))
    Condition
      Error in `clarke_wright()`:
      ! `vehicles$caps` must be positive and finite.
      x `vehicles$caps[2]` is 0.
    Code
      clarke_wright(1:3, distances, data.frame(n = NA, caps = NA))
    Condition
      Error in `clarke_wright()`:
      ! `vehicles$caps` must be positive and finite.
      x `vehicles$caps[1]` is NA.
    Code
      clarke_wright(1:3, distances, unlimited, data.frame(vehicle = 0, site = c(0, 3)))
    Condition
      Error in `clarke_wright()`:
      ! `restrictions$site` must refer to sites of `demand`.
      x `restrictions$site[2]` is 3.
      i Sites are numbered from 0 to 2, in the order of `demand`.
    Code
      clarke_wright(1:3, distances, unlimited, data.frame(vehicle = 1, site = 0))
    Condition
      Error in `clarke_wright()`:
      ! `restrictions$vehicle` must refer to rows of `vehicles`.
      x `restrictions$vehicle[1]` is 1.
      i The only vehicle type is 0.
    Code
      clarke_wright(c(15, 3, 1), distances, data.frame(n = 2L, caps = 6))
    Condition
      Error in `clarke_wright()`:
      ! Not enough vehicles available to start with one run per site.
      x Ran out of vehicles at site 0 (`demand[1]`).
      i The solver merges runs only afterwards, so it can need more vehicles at the start than in the final solution.
      i A vehicle type with an unlimited number (`n = NA`) and without restrictions avoids this.

# Inputs of the wrong type are reported

    Code
      clarke_wright("a", distances, unlimited)
    Condition
      Error in `clarke_wright()`:
      ! `demand` must be a numeric vector, not a string.
    Code
      clarke_wright(1:3, as.matrix(distances), unlimited)
    Condition
      Error in `clarke_wright()`:
      ! `distances` must be a <dist> object, not a double matrix.
      i Create one with `stats::dist()` or `stats::as.dist()`.
    Code
      clarke_wright(1:2, distances, unlimited)
    Condition
      Error in `clarke_wright()`:
      ! `distances` must describe 3 locations: the depot and the 2 sites of `demand`.
      x It describes 4 locations.
    Code
      clarke_wright(1:3, distances, list(n = NA, caps = 6))
    Condition
      Error in `clarke_wright()`:
      ! `vehicles` must be a data frame, not a list.
    Code
      clarke_wright(1:3, distances, data.frame(caps = 6))
    Condition
      Error in `clarke_wright()`:
      ! `vehicles` must have the columns `n` and `caps`.
      x Column `n` is missing.
    Code
      clarke_wright(1:3, distances, data.frame(n = NA, caps = "6"))
    Condition
      Error in `clarke_wright()`:
      ! `vehicles$caps` must be numeric, not a string.
    Code
      clarke_wright(1:3, distances, data.frame(n = 1.5, caps = 6))
    Condition
      Error in `clarke_wright()`:
      ! `vehicles$n` must contain whole numbers.
      x `vehicles$n[1]` is 1.5.
    Code
      clarke_wright(1:3, distances, unlimited, data.frame(site = 0))
    Condition
      Error in `clarke_wright()`:
      ! `restrictions` must have the columns `vehicle` and `site`.
      x Column `vehicle` is missing.
    Code
      clarke_wright(1:3, distances, unlimited, data.frame(site = "a", vehicle = 0))
    Condition
      Error in `clarke_wright()`:
      ! `restrictions$site` must contain whole numbers, not a string.

