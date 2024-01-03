library(purrr)
library(dplyr)
library(tidyr)

set.seed(42)

demand <- runif(500, 5, 15)

pos <-
  data.frame(
    pos_x = c(0, runif(length(demand), -10, 10)),
    pos_y = c(0, runif(length(demand), -10, 10))
  )

vehicles <- data.frame(n = NA_integer_, caps = 33)

result <-
  do.call(
    rbind,
    map(
      # seq(1, 10000, length.out = 2),
      seq(10, 500, by = 20),
      function(n) {
        demand <- demand[1:n]
        distances <- dist(pos[1:(n + 1), ])

        bench::mark(
          clarke_wright(demand, distances, vehicles)
        ) |>
          rowwise() |>
          mutate(
            std = sd(time),
            mean = mean(time),
            n = !!n
          ) |>
          ungroup() |>
          select(n, median, mem_alloc, mean, std)
      },
      .progress = TRUE
    )
  )

saveRDS(result, "./benchmark/results.rds")
