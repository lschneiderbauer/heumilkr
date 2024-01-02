heumilkr_result <- function(df, distances) {
  stopifnot(inherits(df, "data.frame"))
  stopifnot(c("run", "order", "vehicle") %in% colnames(df))
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
