#' Clark-Wright Algorithm (CVRP solver)
#'
#' Finds a quasi-optimal solution to the Capacitated Vehicle Routing
#' Problem (CVRP). It is assumed that all demands can and will be satisfied by a
#' single source.
#'
#' @param demand A numeric vector consisting of "demands" of sites. The length
#'               of the vector equals the number of sites `N` with demands. The
#'               units of the demand values need to match the units of the
#'               vehicle capacity values.
#' @param distances An object of class `dist` (created by [stats::dist()]) with
#'                  `N + 1` locations, describing the distances between sites.
#'                  The zeroth location refers to the source.
#' @param vehicles A data frame describing available vehicle types and
#'                     their respective capacities. One row per vehicle type.
#'                     The data frame is expected to have two columns:
#'                     * `n` - Number of available vehicles. This can be set to
#'                             `NA` if the number is "infinite" (i.e. effectively
#'                             the maximal integer value on your machine.)
#'                     * `caps` - The vehicle capacity in same units as `demand`.
#'                     The order of the data frame is relevant and determines
#'                     the prioritization of vehicle assignments to runs (in case
#'                     two or more vehicle types are eligible for assignment the
#'                     "first" vehicle is chosen).
#' @return Returns a data frame of the same length as `demand` with columns
#'         * `run_id` - Identifies the run the site is assigned to.
#'         * `order`  - Provides the visiting order within each run.
#'         * `vehicle_id` - The vehicle index (as provided in `vehicles`)
#'                          associated to the run.
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
  stopifnot(inherits(distances, "dist"))
  stopifnot(attr(distances, "Size") == length(demand) + 1)
  stopifnot(is.data.frame(vehicles))
  stopifnot(c("n", "caps") %in% colnames(vehicles))
  stopifnot(is.integer(vehicles$n))
  stopifnot(is.numeric(vehicles$caps))

  # replace NAs by maximal machine integer value
  vehicles$n[is.na(vehicles$n)] <- .Machine$integer.max

  heumilkr_result(
    as.data.frame(
      .Call(
        `_heumilkr_cpp_clarke_wright`, as.numeric(demand), distances,
        vehicles$n, vehicles$caps
      ),
      col.names = c("run_id", "order", "vehicle_id")
    ),
    distances = distances
  )
}
