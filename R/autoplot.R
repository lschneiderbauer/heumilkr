#' @importFrom stats cmdscale
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
  data$size <- ifelse(data$site == 0, 5, 3)

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
    geom_path(linewidth = 1, alpha = 0.7) +
    geom_point(
      aes(shape = .data$shape, size = .data$size),
      color = "black"
    ) +
    scale_shape_identity() +
    scale_size_identity() +
    theme_void() +
    theme(legend.position = "none")
}

#' @importFrom graphics plot
#' @export
plot.heumilkr_solution <- function(x, ...) {
  print(autoplot(x, ...))
}

plot_data <- function(x) {
  stopifnot(inherits(x, "heumilkr_solution"))

  # recalculate positions
  pos <- as.data.frame(cmdscale(attr(x, "distances")))
  colnames(pos) <- c("pos_x", "pos_y")
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
          \(y) data.frame(
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
