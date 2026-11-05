heumilkr_solution <- function(dfs, distances) {
  stopifnot(inherits(distances, "dist"))

  new_heumilkr_solution(dfs, distances)
}

new_heumilkr_solution <- function(dfs, distances) {  
  structure(
    dfs,
    distances = distances,
    class = c("heumilkr_solution", class(dfs))
  )
}

#' @export
print.heumilkr_solution <- function(x, ...) {
  print(c(x)) # misuse c() to strip attributes
}

#' Vehicle runs cost / distance
#'
#' Calculates the total distance associated to a [clarke_wright()] result.
#' This is the measure that the corresponding Capacitated Vehicle Routing
#' Problem minimizes.
#'
#' @param solution
#'  A "`heumilkr_solution`" object, typically obtained by [clarke_wright()].
#'
#' @return The total traveled distance.
#' @examples
#' demand <- c(3, 2, 4, 2)
#'
#' positions <-
#'   data.frame(
#'     pos_x = c(0, 1, -1, 2, 3),
#'     pos_y = c(0, 1, 1, 2, 3)
#'   )
#'
#' solution <- clarke_wright(
#'   demand,
#'   dist(positions),
#'   data.frame(n = NA_integer_, caps = 6)
#' )
#'
#' milkr_cost(solution)
#'
#' @export
milkr_cost <- function(solution) {
  stopifnot(inherits(solution, "heumilkr_solution"))

  sum(solution$runs[, "distance"])
}

#' Vehicle run saving
#'
#' Measures the saving that was achieved by the heuristic optimization
#' algorithm [clarke_wright()] compared to the naive vehicle run assignment,
#' i.e. one run per site, or as many runs as the solution uses for a site whose
#' demand exceeds the vehicle capacities.
#'
#' @param solution
#'  A "`heumilkr_solution`" object, typically obtained by [clarke_wright()].
#' @param relative
#'  Should the saving be given as dimensionful value (in units of distance as
#'  provided to [clarke_wright()]), or as percentage relative to the
#'  naive costs.
#'  Defaults to `FALSE`, i.e. a dimensionful value.
#'
#' @return
#'  The savings either as dimensionful value or as percentage relative to the
#'  naive costs, depending on `relative`.
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
#' solution <- clarke_wright(
#'   demand,
#'   dist(positions),
#'   data.frame(n = NA_integer_, caps = 6)
#' )
#'
#' print(milkr_saving(solution))
#' print(milkr_saving(solution, relative = TRUE))
#'
#' @export
milkr_saving <- function(solution, relative = FALSE) {
  stopifnot(inherits(solution, "heumilkr_solution"))
  stopifnot(is.logical(relative))

  d <- as.matrix(attr(solution, "distances"))

  # a round trip for each visit: a site with split demand has multiple visits
  naive_cost <- sum(2 * d[1, solution$visits$site + 2])

  saving <- naive_cost - milkr_cost(solution)

  if (relative) {
    saving / naive_cost
  } else {
    saving
  }
}

milkr_perf_scale <- function(solution, best_known_cost) {
  stopifnot(inherits(solution, "heumilkr_solution"))
  stopifnot(is.numeric(best_known_cost))

  cost <- milkr_cost(solution)
  saving <- milkr_saving(solution)

  # measures where we are on the scale between best known and naive solution
  # (= 0 when cost = naive cost, = 1 when cost = best known cost)
  1 - (cost - best_known_cost) / (cost - best_known_cost + saving)
}

milkr_perf_rel <- function(solution, best_known_cost) {
  stopifnot(inherits(solution, "heumilkr_solution"))
  stopifnot(is.numeric(best_known_cost))

  cost <- milkr_cost(solution)

  # how much better is the best known solution compared to this one
  (cost - best_known_cost) / cost
}
