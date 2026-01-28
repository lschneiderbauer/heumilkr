#' Clarke-Wright algorithm, a Capacitated Vehicle Routing Problem solver
#'
#' Finds a quasi-optimal solution to the Capacitated Vehicle Routing
#' Problem (CVRP). It is assumed that all demands will be satisfied by a
#' single source (i.e. "depot") which is the origin of all runs.
#'
#' @details
#' See the original paper,
#' Clarke, G. and Wright, J.R. (1964) \doi{10.1287/opre.12.4.568},
#' for a detailed explanation of the Clarke-Wright algorithm.
#'
#' @param demand
#'  A [numeric()] vector consisting of "demands" indexed by sites.
#'  The `i`th entry refers to the demand of site `i` that is requested by the
#'  source depot (origin).
#'  The length of the vector equals the number of sites `N`. The
#'  units of demand values need to match the units of vehicle capacity values.
#'  `NA` values are not allowed. Negative demand values are also allowed and are
#'  interpreted as freight that needs to be delivered from a site
#'  to the source source depot (origin), i.e. "backhaul". Note that sites with
#'  negative demands can only be visited after after sites with positive demand.
#'
#' @param distances
#'  An object of class `dist`, created by [stats::dist()], with
#'  `(N + 1)` locations describing the distances between individual
#'  sites. The first index refers to the source location. The `(i+1)`th
#'  index refers to site `i` (as defined by `demand`).
#'
#' @param vehicles
#'  A [data.frame()] describing available vehicle types and their respective
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
#'  The order of the [data.frame()] is relevant and determines the prioritization
#'  of vehicle assignments to runs (in case two or more vehicle types are
#'  eligible for assignment the "first" vehicle is chosen). In a typical scenario
#'  "more expensive" vehicles should be further down in the list (so the cheaper
#'  one is chosen in case there is doubt). Since higher capacity vehicles
#'  usually involve higher costs sorting the data frame by capacity is usually
#'  a good rule of thumb.
#'
#' @param restrictions
#'  An optional [data.frame()] that allows to define vehicle type restrictions for
#'  particular sites in the form of a blacklist.
#'  The data frame is expected to have two columns:
#'  * `vehicle` - The vehicle type index.
#'  * `site` - The site index (i.e. the index of the `demand` vector)
#'
#'  Each row defines a restriction: vehicle type `vehicle` can not approach site
#'  `site`. Defaults to an empty [data.frame()], i.e. no restrictions are enforced.
#'
#' @return
#'  Returns a "`heumilkr_solution`" object, a named [list()] of three [data.frame()]s
#'  with names "runs", "sites" and "visits", bestowed with additional attributes.
#'  * `runs`: Each record reflects a single run/tour.
#'    * `run` - A run identifier.
#'    * `vehicle` - The vehicle type index (as provided in `vehicles`) associated
#'                to the run.
#'    * `max_load` - The maximal load of the vehicle on this run. It is always
#'                  smaller or equal the vehicle capacity.
#'    * `distance` - The travel distance of the particular run.
#'  * `sites`: Each record reflect a site with demand. Its columns consist of:
#'    * `site` - A site identifier.
#'    * `demand` - The site demand. This is supplied as an input.
#'  * `visits`: Each record reflects a run visit of a particular site.
#'  It is implied that each run starts and ends at the origin.
#'  Its columns consist of:
#'    * `run` - Identifies the run the site is assigned to.
#'    * `site` - The site index (i.e. the index of the (1-indexed) `demand` vector)
#'              associated to the run.
#'    * `order` - Integer values providing the visiting order within each run.
#'    * `load` - The departing load on site `site` in units of `demand` per particular run.
#'  
#' 
#'  Unless a site demand exceeds the vehicle capacities it is always assigned
#'  to only a single run.
#'
#' @examples
#' demand <- c(3, 2, 4, 2)
#'
#' positions <-
#'   data.frame(
#'     pos_x = c(0, 1, -1, 2, 3),
#'     pos_y = c(0, 1, 1, 2, 3)
#'   )
#'
#' clarke_wright(
#'   demand,
#'   dist(positions),
#'   data.frame(n = NA_integer_, caps = 6)
#' )
#'
#' @export
clarke_wright <- function(
  demand,
  distances,
  vehicles,
  restrictions = data.frame(
    vehicle = integer(),
    site = integer()
  )
) {
  stopifnot(is.numeric(demand))
  stopifnot(all(!is.na(demand)))
  stopifnot(inherits(distances, "dist"))
  stopifnot(attr(distances, "Size") == length(demand) + 1)
  stopifnot(all(!is.na(distances)))
  stopifnot(is.data.frame(vehicles))
  stopifnot(c("n", "caps") %in% colnames(vehicles))
  stopifnot(nrow(vehicles) > 0)
  stopifnot(is.integer(vehicles$n))
  stopifnot(is.numeric(vehicles$caps))
  stopifnot(vehicles$caps > 0)
  stopifnot(is.data.frame(restrictions))
  stopifnot(c("site", "vehicle") %in% colnames(restrictions))

  # replace NAs by maximal machine integer value
  vehicles$n[is.na(vehicles$n)] <- .Machine$integer.max

  heumilkr_solution(
    .Call(
      `_heumilkr_r_cpp_clarke_wright`,
      as.numeric(demand),
      distances,
      vehicles$n,
      vehicles$caps,
      restrictions$site,
      restrictions$vehicle
    ),
    distances = distances
  )
}

#' Stepwise Clarke-Wright algorithm, a Capacitated Vehicle Routing
#' Problem solver
#'
#' Same as [clarke_wright()] but returns all intermediate state results as well.
#' This function exists only for showcase purposes and should not be used in
#' production (performance is quite bad).
#'
#' @inheritParams clarke_wright
#' @seealso [clarke_wright()]
#' @noRd
clarke_wright_stepwise <- function(
  demand,
  distances,
  vehicles,
  restrictions = data.frame(
    vehicle = integer(),
    site = integer()
  )
) {
  stopifnot(is.numeric(demand))
  stopifnot(all(!is.na(demand)))
  stopifnot(inherits(distances, "dist"))
  stopifnot(attr(distances, "Size") == length(demand) + 1)
  stopifnot(is.data.frame(vehicles))
  stopifnot(c("n", "caps") %in% colnames(vehicles))
  stopifnot(nrow(vehicles) > 0)
  stopifnot(is.integer(vehicles$n))
  stopifnot(is.numeric(vehicles$caps))
  stopifnot(vehicles$caps > 0)
  stopifnot(is.data.frame(restrictions))
  stopifnot(c("site", "vehicle") %in% colnames(restrictions))

  # replace NAs by maximal machine integer value
  vehicles$n[is.na(vehicles$n)] <- .Machine$integer.max

  lapply(
    .Call(
      `_heumilkr_cpp_clarke_wright_stepwise`,
      as.numeric(demand),
      distances,
      vehicles$n,
      vehicles$caps,
      restrictions$site,
      restrictions$vehicle
    ),
    function(x) {
      heumilkr_solution(
        x,
        distances = distances
      )
    }
  )
}
