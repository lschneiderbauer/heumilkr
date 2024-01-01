#' @importFrom stats cmdscale
#' @importFrom ggplot2 autoplot ggplot aes geom_point geom_path
#'                              scale_color_discrete
#'                              scale_shape_identity
#'                              scale_size_identity
#'                              theme_void theme
#' @importFrom rlang .data
#' @export
autoplot.heumilkr_result <- function(res, ...) {

  # recalculate positions
  pos <- as.data.frame(cmdscale(attr(res, "distances")))
  colnames(pos) <- c("pos_x", "pos_y")
  pos$site <- seq.int(nrow(pos)) - 1

  runs <-
    rbind(
      data.frame(
        run_id = unique(res$run_id),
        order = -1,
        site = 0
      ),
      cbind(res[, c("run_id", "order")], site = seq.int(nrow(res))),
      do.call(rbind, by(
        res,
        res$run_id,
        \(x) data.frame(
          run_id = unique(x$run_id),
          order = 1 + max(x$order),
          site = 0
        )
      ))
    )

  data <- merge(runs, pos, by = "site")
  data$shape <- ifelse(data$site == 0, "square", "circle")
  data$size <- ifelse(data$site == 0, 5, 3)

  ggplot(
    data = data[order(data$run_id, data$order), ],
    aes(
      x = .data$pos_x, y = .data$pos_y,
      group = .data$run_id
    )
  ) +
    scale_color_discrete() +
    geom_path(
      aes(color = as.factor(.data$run_id)),
      linewidth = 1, alpha = 0.7
    ) +
    geom_point(aes(shape = .data$shape, size = .data$size)) +
    scale_shape_identity() +
    scale_size_identity() +
    theme_void() +
    theme(legend.position = "none")
}

#' @importFrom graphics plot
#' @export
plot.heumilkr_result <- function(x, ...) {
  print(autoplot(x, ...))
}
