heumilkr_solution <- function(df, distances) {
  stopifnot(inherits(df, "data.frame"))
  stopifnot(
    c("site", "run", "order", "vehicle", "order", "distance") %in% colnames(df)
  )
  stopifnot(inherits(distances, "dist"))

  new_heumilkr_solution(df, distances)
}

new_heumilkr_solution <- function(df, distances) {
  obj <-
    structure(
      df,
      class = c("heumilkr_solution", class(df))
    )

  attr(obj, "distances") <- distances
  obj
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

  sum(unique(solution[, c("run", "distance")])$distance)
}

#' Vehicle run saving
#'
#' Measures the saving that was achieved by the heuristic optimization
#' algorithm [clarke_wright()] compared to the naive vehicle run assignment,
#' i.e. one run per site.
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

  naive_cost <-
    sum(
      vapply(
        1:(dim(d)[[2]] - 1),
        \(idx) 2 * d[1, idx + 1],
        FUN.VALUE = 1.
      )
    )

  saving <- naive_cost - milkr_cost(solution)

  if (relative) {
    saving / naive_cost
  } else {
    saving
  }
}
