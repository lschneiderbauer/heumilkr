#' Clark-Wright Algorithm (CVRP solver)
#'
#' Finds a quasi-optimal solution to the Capacitated Vehicle Routing
#' Problem (CVRP). It is assumed that all demands will be satisfied by a
#' single source.
#'
#' @details
#' See the original paper,
#' [Clarke, G. and Wright, J.R. (1964)](http://dx.doi.org/10.1287/opre.12.4.568),
#' for details.
#'
#' @param demand
#'  A numeric vector consisting of "demands" indexed by sites.
#'  The `i`th entry refers to the demand of site `i` (and the length
#'  of the vector equals the number of sites `N` with demands). The
#'  units of the demand values need to match the units of the
#'  vehicle capacity values.
#'
#' @param distances
#'  An object of class `dist`, created by [stats::dist()], with
#'  `(N + 1)` locations describing the distances between individual
#'  sites. The first index refers to the source site. The `(i+1)`'th
#'  index refers to site `i` (as defined by `demand`).
#'
#' @param vehicles
#'  A data frame describing available vehicle types and their respective
#'  capacities. One row per vehicle type. The data frame is expected to have
#'  two columns:
#'  * `n` - Number of available vehicles. This can be set to `NA` if the
#'          number is "infinite" (i.e. effectively the maximal integer value
#'          on your machine.).
#'          It is recommended to keep at least one vehicle type as "infinite",
#'          otherwise the solver might raise a run time error due to initially
#'          not having enough vehicles available (even though the final
#'          solution might satisfy the availability restrictions).
#'  * `caps` - The vehicle capacity in same units as `demand`.
#'
#'  The order of the data frame is relevant and determines the prioritization
#'  of vehicle assignments to runs (in case two or more vehicle types are
#'  eligible for assignment the "first" vehicle is chosen).
#'
#' @return
#'  Returns a data frame with one row per site-run combination.
#'  * `site` - The site index (as provided in `demand`) associated
#'             to the run.
#'  * `run` - Identifies the run the site is assigned to.
#'  * `order`  - Integer values providing the visiting order within each run.
#'  * `vehicle` - The vehicle index (as provided in `vehicles`) associated
#'                to the run.
#'
#'  Unless a site demand exceeds the vehicle capacities it is always assigned
#'  to only a single run.
#'
#' @export
#'
#' @examples
#' demand <- c(3, 2, 4, 2)
#'
#' pos <-
#'   data.frame(
#'     pos_x = c(0, 1, -1, 2, 3),
#'     pos_y = c(0, 1, 1, 2, 3)
#'   )
#'
#' clarke_wright(
#'   demand,
#'   dist(pos),
#'   data.frame(n = NA_integer_, caps = 6)
#' )
clarke_wright <- function(demand, distances, vehicles) {
  stopifnot(is.numeric(demand))
  stopifnot(inherits(distances, "dist"))
  stopifnot(attr(distances, "Size") == length(demand) + 1)
  stopifnot(is.data.frame(vehicles))
  stopifnot(c("n", "caps") %in% colnames(vehicles))
  stopifnot(nrow(vehicles) > 0)
  stopifnot(is.integer(vehicles$n))
  stopifnot(is.numeric(vehicles$caps))
  stopifnot(vehicles$caps > 0)

  # replace NAs by maximal machine integer value
  vehicles$n[is.na(vehicles$n)] <- .Machine$integer.max

  heumilkr_result(
    .Call(
      `_heumilkr_cpp_clarke_wright`, as.numeric(demand), distances,
      vehicles$n, vehicles$caps
    ),
    distances = distances
  )
}

#' Same as [clarke_wright()] but returns all intermediate state results as well.
#' This function exists only for showcase purposes and should not be used in
#' production (performance is quite bad).
#'
#' @inheritParams clarke_wright
#' @seealso [clarke_wright()]
#' @noRd
clarke_wright_stepwise <- function(demand, distances, vehicles) {
  stopifnot(inherits(distances, "dist"))
  stopifnot(attr(distances, "Size") == length(demand) + 1)
  stopifnot(is.data.frame(vehicles))
  stopifnot(c("n", "caps") %in% colnames(vehicles))
  stopifnot(is.integer(vehicles$n))
  stopifnot(is.numeric(vehicles$caps))

  # replace NAs by maximal machine integer value
  vehicles$n[is.na(vehicles$n)] <- .Machine$integer.max

  lapply(
    .Call(
      `_heumilkr_cpp_clarke_wright_stepwise`, as.numeric(demand), distances,
      vehicles$n, vehicles$caps
    ),
    \(x) heumilkr_result(
      x, distances = distances
    )
  )
}
