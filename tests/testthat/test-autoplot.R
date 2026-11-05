test_that("autoplot does not err", {
  skip_if_not_installed("hedgehog")

  hedgehog::forall(
    gen.demand_net(max_sites = 10L),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances,
          vehicles = data.frame(n = NA_integer_, caps = 99999)
        )

      expect_no_error(
        pl <- autoplot(res)
      )

      expect_s3_class(pl, "ggplot")
    }
  )
})

test_that("plots place visits correctly when demand is split across runs", {
  distances <- dist(data.frame(x = c(0, 1, 2, -1), y = c(0, 1, 0, 1)))
  vehicles <- data.frame(n = NA_integer_, caps = 30)

  # the number of visits can be a multiple of the number of sites, or not
  for (demand in list(c(61, 5, 5), c(100, 5, 5), c(-61, 5, 5))) {
    res <- clarke_wright(demand, distances, vehicles)

    expect_no_error(pl <- autoplot(res))
    expect_s3_class(pl, "ggplot")

    plot_visits <- plot_data(res)
    plot_visits <- plot_visits[plot_visits$site != 0, ]
    plot_visits <- plot_visits[order(plot_visits$run, plot_visits$order), ]
    visits <- res$visits[order(res$visits$run, res$visits$order), ]

    expect_equal(plot_visits$run, visits$run)
    expect_equal(plot_visits$site, visits$site + 1)
  }
})
