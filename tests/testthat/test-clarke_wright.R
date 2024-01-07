test_that("runs without error", {
  demand <- c(1, 1)

  pos <-
    data.frame(
      pos_x = c(0, 1, -1),
      pos_y = c(0, 1, 1)
    )

  expect_no_error(
    clarke_wright(
      demand,
      dist(pos),
      data.frame(n = NA_integer_, caps = 99999)
    )
  )
})

test_that("Sum of loads over all runs equals sum of demands", {
  skip_if_not_installed("hedgehog")

  hedgehog::forall(
    gen.demand_net(max_sites = 10L),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances, data.frame(n = c(NA_integer_, 3L), caps = c(60, 120))
        )

      expect_equal(
        sum(unique(data.frame(res$run, res$load))$res.load),
        sum(demand_net$demand)
      )
    }
  )
})

test_that("Distances add up correctly.", {
  demand <- rep(1, 10)

  pos <-
    data.frame(
      pos_x = 0:10,
      pos_y = 0
    )

  res <- clarke_wright(
    demand, dist(pos),
    data.frame(n = NA_integer_, caps = 1)
  )

  expect_equal(res$distance, 1:10 * 2)
})

test_that("Limited vehicles with more priority should always be exhausted
           provided there is enough demand", {
  skip_if_not_installed("hedgehog")

  hedgehog::forall(
    gen.demand_net(max_sites = 10L),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances,
          data.frame(
            n = c(3L, NA_integer_),
            caps = c(66, 33)
          )
        )
      # note: we deliberately put the higher capacity vehicle first,
      # so this one always gets chosen.

      expect_equal(
        nrow(unique(res[res$vehicle == 0, ][, c("run", "vehicle")])),
        pmin(length(unique(res$run)), 3)
      )
    }
  )
})

test_that("A vehicle with infinite capacity covers everything in a single run", {
  skip_if_not_installed("hedgehog")

  hedgehog::forall(
    gen.demand_net(max_sites = 10L),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances, data.frame(n = NA_integer_, caps = 99999)
        )

      expect_equal(unique(res$run), 0)
      expect_equal(unique(res$vehicle), 0)
    }
  )
})

test_that("A demand that exceeds vehicle capacities generates more than a single run", {
  demand <- c(15)

  pos <-
    data.frame(
      pos_x = c(0, 0),
      pos_y = c(0, 1)
    )

  res <-
    clarke_wright(
      demand,
      dist(pos),
      data.frame(n = NA_integer_, caps = 6)
    )

  expect_equal(length(unique(res$run)), 3)
  expect_equal(unique(res$site), 0)
  expect_equal(sort(res$load), c(3, 6, 6))
  expect_equal(res$distance, c(2, 2, 2))
})


test_that("Vehicles are not assigned to restricted sites", {
  skip_if_not_installed("hedgehog")

  hedgehog::forall(
    gen.demand_net(max_sites = 10L),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances,
          vehicles = data.frame(n = c(NA_integer_, NA_integer_), caps = c(99999, 99999)),
          restrictions = data.frame(site = 0L, vehicle = 0L)
        )

      expect_false(
        0 %in% res[res$site == 0]$vehicle
      )
    }
  )
})


test_that("Not having enough vehicles is handled gracefully", {
  demand <- c(15, 3, 1)

  pos <-
    data.frame(
      pos_x = c(0, 1, -1, -2),
      pos_y = c(0, 1, 1, 2)
    )

  expect_error(
    clarke_wright(
      demand,
      dist(pos),
      data.frame(n = 2L, caps = 6)
    )
  )
})

test_that("Having NA demand values aborts", {
  demand <- c(15, 3, NA)

  pos <-
    data.frame(
      pos_x = c(0, 1, -1, -2),
      pos_y = c(0, 1, 1, 2)
    )

  expect_error(
    clarke_wright(
      demand,
      dist(pos),
      data.frame(n = NA, caps = 6)
    )
  )
})

test_that("README example result is preserved.", {
  set.seed(42)
  demand <- runif(20, 5, 15)

  pos <-
    data.frame(
      pos_x = c(0, runif(length(demand), -10, 10)),
      pos_y = c(0, runif(length(demand), -10, 10))
    )

  res <-
    clarke_wright(
      demand,
      dist(pos),
      data.frame(n = c(NA_integer_, 2L), caps = c(33, 44))
    )

  expect_snapshot(res)
})
