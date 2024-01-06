heumilkr_result <- function(df, distances) {
  stopifnot(inherits(df, "data.frame"))
  stopifnot(
    c("site", "run", "order", "vehicle", "order", "distance") %in% colnames(df)
  )
  stopifnot(inherits(distances, "dist"))

  new_heumilkr_result(df, distances)
}

new_heumilkr_result <- function(df, distances) {
  obj <-
    structure(
      df,
      class = c("heumilkr_result", class(df))
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
#' @param res
#'  A "`heumilkr_result`" object, typically obtained by [clarke_wright()].
#'
#' @return The total traveled distance.
#' @examples
#' demand <- c(3, 2, 4, 2)
#'
#' pos <-
#'   data.frame(
#'     pos_x = c(0, 1, -1, 2, 3),
#'     pos_y = c(0, 1, 1, 2, 3)
#'   )
#'
#' res <- clarke_wright(
#'   demand,
#'   dist(pos),
#'   data.frame(n = NA_integer_, caps = 6)
#' )
#'
#' milkr_cost(res)
#'
#' @export
milkr_cost <- function(res) {
  stopifnot(inherits(res, "heumilkr_result"))

  sum(unique(res[, c("run", "distance")])$distance)
}
