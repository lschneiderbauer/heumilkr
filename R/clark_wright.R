#' Clark-Wright Algorithm (CVRP solver)
#'
#' Finds a quasi-optimal solution to the Capacity Vehicle Routing
#' problem (CVRP). It is assumed that all demands can and will be satisfied by a
#' single source.
#'
#' @param demand A numeric vector consisting of "demands" of sites. The length
#'               of the vector equals the number of sites `n` with demands. The
#'               units of the demand values need to match the units of the
#'               vehicle capacity values.
#' @param distances An object of class `dist` (created by [stats::dist()]) with
#'                  `n + 1` locations, describing the distances between sites.
#'                  The zeroth location refers to the source.
#'
#' @return
#' @export
#'
#' @examples
clark_wright <- function(demand, distances, vehicle_caps) {
  stopifnot(inherits(distances, "dist"))
  stopifnot(attr(distances, "Size") == length(demand) + 1)
  stopifnot(is.data.frame(vehicle_caps))
  stopifnot(c("n", "caps") %in% colnames(vehicle_caps))
  stopifnot(is.integer(vehicle_caps$n))
  stopifnot(is.numeric(vehicle_caps$caps))

  # replace NAs by maximal machine integer value
  vehicle_caps$n[is.na(vehicle_caps$n)] <- .Machine$integer.max

  as.data.frame(
    .Call(
      `_heumilkr_cpp_clark_wright`, as.vector(demand), distances,
      vehicle_caps$n, vehicle_caps$caps
    ),
    col.names = c("run_id", "order")
  )
}
