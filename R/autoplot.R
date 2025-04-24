#' Create ggplot for a CVRP solution
#'
#' @description
#' Represents the sites and runs on a 2D plane so that the distances between
#' sites on the drawn 2D plane correspond to `distances` provided to the
#' solver `clarke_wright()`.
#'
#' The individual runs are distinguished by color.
#' The demanding site locations are marked with round circles while the
#' (single) supplying site is depicted as a square.
#' The line types (solid/dashed/...) are associated to different vehicle types.
#'
#' @details
#' Distance information between sites only determine
#' site positions on a 2D plane up to rotations and translations:
#' those are fixed arbitrarily.
#'
#' @param object
#'  A "`heumilkr_solution`" object, typically obtained by [clarke_wright()].
#'
#' @param ... Not used.
#'
#' @return A ggplot object.
#'
#' @importFrom ggplot2 autoplot ggplot aes geom_point geom_path
#'                              scale_color_discrete
#'                              scale_shape_identity
#'                              scale_size_identity
#'                              theme_void theme
#' @importFrom rlang .data
#' @export
autoplot.heumilkr_solution <- function(object, ...) {
  data <- plot_data(object)

  data$shape <- ifelse(data$site == 0, "square", "circle")
  data$size <- ifelse(data$site == 0, 4, 1.5)

  ggplot(
    data = data[order(data$run, data$order), ],
    aes(
      x = .data$pos_x, y = .data$pos_y,
      color = as.factor(.data$run),
      group = .data$run,
      linetype = as.factor(.data$vehicle)
    )
  ) +
    scale_color_discrete() +
    geom_path(linewidth = 0.8, alpha = 0.7) +
    geom_point(
      aes(shape = .data$shape, size = .data$size),
      color = "black"
    ) +
    scale_shape_identity() +
    scale_size_identity() +
    theme_void() +
    theme(legend.position = "none")
}

#' Plot a CVRP solution
#'
#' @inherit autoplot.heumilkr_solution description
#' @inherit autoplot.heumilkr_solution details
#'
#' @param x
#'  A "`heumilkr_solution`" object, typically obtained by [clarke_wright()].
#' @inheritParams autoplot.heumilkr_solution
#'
#' @importFrom graphics plot
#' @export
plot.heumilkr_solution <- function(x, ...) {
  print(autoplot(x, ...))
}

#' @importFrom stats cmdscale
plot_data <- function(x) {
  stopifnot(inherits(x, "heumilkr_solution"))

  # recalculate positions
  if (length(attr(x, "distances")) <= 1) {
    # special case when we only have two positions:
    # effectively a one dimensional problem
    # we embed it in 2D in the y = 0 line.
    pos <- as.data.frame(cmdscale(attr(x, "distances"), k = 1))
    colnames(pos) <- c("pos_x")
    pos$pos_y <- 0
  } else {
    pos <- as.data.frame(cmdscale(attr(x, "distances"), k = 2))
    colnames(pos) <- c("pos_x", "pos_y")
  }
  pos$site <- seq.int(nrow(pos)) - 1

  runs <-
    rbind(
      data.frame(
        run = unique(x$run),
        order = -1,
        site = 0
      ),
      cbind(x[, c("run", "order")], site = seq.int(nrow(x))),
      do.call(
        rbind,
        by(
          x,
          x$run,
          function(y) data.frame(
            run = unique(y$run),
            order = 1 + max(y$order),
            site = 0
          )
        )
      )
    )

  merge(
    merge(runs, pos, by = "site"),
    x[, c("run", "vehicle")],
    "run"
  )
}
