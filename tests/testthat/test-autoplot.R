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
