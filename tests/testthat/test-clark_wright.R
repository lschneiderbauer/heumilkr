test_that("runs without error", {
  demand <- c(1, 1)

  pos <-
    data.frame(
      pos_x = c(0, 1, -1),
      pos_y = c(0, 1, 1)
    )

  expect_no_error(
    clark_wright(
      demand,
      dist(pos),
      data.frame(n = NA_integer_, caps = 99999)
    )
  )
})

test_that("vehicle with infinite capacity covers everything in a single run", {
  hedgehog::forall(
    gen.demand_net(max_sites = 10L),
    function(demand_net) {
      res <-
        clark_wright(
          demand_net$demand,
          demand_net$distances, data.frame(n = NA_integer_, caps = 99999)
        )

      expect_equal(length(unique(res$run_id)), 1)
    }
  )
})
