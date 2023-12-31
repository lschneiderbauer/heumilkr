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
