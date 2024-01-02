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

test_that("A vehicle with infinite capacity covers everything in a single run", {
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
      pos_x = c(0, 1),
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
      data.frame(n = NA_integer_, caps = 33)
    )

  expect_snapshot(res)
})
