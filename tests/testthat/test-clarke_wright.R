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

test_that("every site is represented in output", {
  demand <- c(1, 1)

  pos <-
    data.frame(
      pos_x = c(0, 1, -1),
      pos_y = c(0, 1, 1)
    )

  res <-
    clarke_wright(
      demand,
      dist(pos),
      data.frame(n = NA_integer_, caps = 99999)
    )

  expect_equal(
    sort(unique(res$sites$site)),
    seq_along(demand) - 1
  )
})

test_that("Sum of loads over all runs equals sum of demands", {
  skip_if_not_installed("hedgehog")

  hedgehog::forall(
    gen.demand_net(max_sites = 10L, min_demand = 1),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances,
          data.frame(n = c(NA_integer_, 3L), caps = c(60, 120))
        )

      expect_equal(
        sum(res$runs$max_load),
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
    demand,
    dist(pos),
    data.frame(n = NA_integer_, caps = 1)
  )

  expect_equal(sort(res$runs$distance), 1:10 * 2)
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
        nrow(res$runs[res$runs$vehicle == 0, ]),
        pmin(length(res$runs$run), 3)
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
          demand_net$distances,
          data.frame(n = NA_integer_, caps = 99999)
        )

      expect_equal(res$runs$run, 0)
      expect_equal(res$runs$vehicle, 0)
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

  expect_equal(length(res$runs$run), 3)
  expect_equal(res$sites$site, 0)
  expect_equal(sort(res$runs$max_load), c(3, 6, 6))
  expect_equal(res$runs$distance, c(2, 2, 2))
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
          vehicles = data.frame(
            n = c(NA_integer_, NA_integer_),
            caps = c(99999, 99999)
          ),
          restrictions = data.frame(site = 0L, vehicle = 0L)
        )

      runs <- res$visits[res$visits$site == 0, "run"]

      expect_false(
        0 %in% res$runs[res$runs$run %in% runs, "vehicle"]
      )
    }
  )
})


test_that("Vehicles are not assigned to restricted sites: edge case", {
  res <-
    clarke_wright(
      demand = c(6.0, 8.0, 6.5, 11.5, 5.5),
      distances = as.dist(
        matrix(
          c(
            0.000,
            44.2920,
            3.554,
            41.088,
            32.0590,
            28.355,
            44.292,
            0.0000,
            46.886,
            87.104,
            81.9915,
            18.932,
            3.554,
            46.8860,
            0.000,
            36.731,
            27.7560,
            30.949,
            41.088,
            87.1040,
            36.731,
            0.000,
            7.7130,
            67.273,
            32.059,
            81.9915,
            27.756,
            7.713,
            0.0000,
            58.298,
            28.355,
            18.9320,
            30.949,
            67.273,
            58.2980,
            0.000
          ),
          nrow = 6
        )
      ),
      vehicles = data.frame(
        caps = c(38, 33),
        n = NA_integer_
      ),
      restrictions = data.frame(
        site = 3L,
        vehicle = 0L
      )
    )

  runs <- res$visits[res$visits$site == 0, "run"]
  expect_false(0 %in% res$runs[res$run %in% runs, "vehicle"])
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

test_that("Having NA dist values aborts", {
  demand <- 3
  dist_mat <- c(0, NA, NA, 0) # note those NAs here
  dim(dist_mat) <- c(2, 2)

  expect_error(
    clarke_wright(
      demand,
      as.dist(dist_mat),
      data.frame(n = NA_integer_, caps = 6)
    )
  )
})

test_that("Example that previously caused integer overflow does not crash anymore", {
  f_demand <- 399
  dist_matrix <-
    structure(
      17.84195,
      Labels = c("100402", "40238"),
      Size = 2L,
      class = "dist",
      Diag = FALSE,
      Upper = FALSE
    )

  vehicle_types <-
    data.frame(
      n = c(NA_integer_, NA_integer_),
      caps = c(33, 38)
    )

  expect_no_error(
    clarke_wright(f_demand, dist_matrix, vehicle_types)
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

  expect_lte(
    milkr_cost(res),
    170.2523
  )
})

test_that("Truck loads are always within physical boundaries at any point on the run (even with negative demands)", {
  skip_if_not_installed("hedgehog")

  hedgehog::forall(
    gen.demand_net(max_sites = 10L),
    function(demand_net) {
      max_cap <- 120

      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances,
          data.frame(n = c(NA_integer_, 3L), caps = c(60, max_cap))
        )

      expect_all_true(res$visits$load >= 0)
      expect_all_true(res$visits$load <= max_cap)
    }
  )
})

test_that("Single-signed demand: Max of demand sum for each run equals max_load", {
  skip_if_not_installed("hedgehog")

  hedgehog::forall(
    gen.demand_net(max_sites = 10L, min_demand = 1L),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances,
          data.frame(n = c(NA_integer_, 3L), caps = c(60, 120))
        )

      res1 <- merge(
        res$visits,
        res$sites,
        by = "site"
      )

      load <-
        as.numeric(
          by(res1, res1$run, function(x) sum(x$demand))
        )

      expect_equal(
        res$runs$max_load,
        abs(load)
      )
    }
  )

  hedgehog::forall(
    gen.demand_net(max_sites = 10L, max_demand = -1L),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances,
          data.frame(n = c(NA_integer_, 3L), caps = c(60, 120))
        )

      res1 <- merge(
        res$visits,
        res$sites,
        by = "site"
      )

      load <-
        as.numeric(
          by(res1, res1$run, function(x) sum(x$demand))
        )

      expect_equal(
        res$runs$max_load,
        abs(load)
      )
    }
  )
})


test_that("Example scenario with negative demand yields a single run", {
  res <- clarke_wright(
    demand = c(10, 10, 10, -5),
    distances = stats::dist(
      data.frame(
        x = c(0, 1, 2, 3, 4),
        y = c(0, 0, 1, 0, 1)
      )
    ),
    vehicles = data.frame(n = NA_integer_, caps = 30)
  )

  expect_equal(length(res$runs$run), 1)
})


test_that("Last result of clarke_wright stepwise should be the original result", {
  skip_if_not_installed("hedgehog")

  # requirement for that to be true:
  # * only count positive demands
  # * demand is always <= vehicle capacity
  hedgehog::forall(
    gen.demand_net(max_sites = 10L),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances,
          data.frame(n = c(NA_integer_, 3L), caps = c(60, 120))
        )

      res_sw <-
        clarke_wright_stepwise(
          demand_net$demand,
          demand_net$distances,
          data.frame(n = c(NA_integer_, 3L), caps = c(60, 120))
        )

      expect_true(is.list(res_sw))
      expect_equal(res_sw[[length(res_sw)]], res)
    }
  )
})

# Recomputes distance and load profile of each run from the visits table.
# Only valid if no site demand is split across multiple runs.
run_audit <- function(res, demand, distances) {
  m <- as.matrix(distances)

  do.call(
    rbind,
    lapply(
      res$runs$run,
      function(r) {
        visits <- res$visits[res$visits$run == r, ]
        visits <- visits[order(visits$order), ]
        idx <- visits$site + 2
        site_demand <- demand[visits$site + 1]
        initial_load <- sum(pmax(site_demand, 0))
        load <- initial_load - cumsum(site_demand)

        data.frame(
          run = r,
          distance = m[1, idx[1]] +
            sum(m[cbind(idx[-length(idx)], idx[-1])]) +
            m[idx[length(idx)], 1],
          max_load = max(initial_load, load),
          load_ok = isTRUE(all.equal(visits$load, load))
        )
      }
    )
  )
}

test_that("Reported distances match the visiting order", {
  skip_if_not_installed("hedgehog")

  vehicles <- data.frame(n = NA_integer_, caps = 60)

  check <- function(demand_net) {
    res <- clarke_wright(demand_net$demand, demand_net$distances, vehicles)
    audit <- run_audit(res, demand_net$demand, demand_net$distances)

    expect_equal(res$runs$distance, audit$distance)
  }

  hedgehog::forall(gen.demand_net(max_sites = 10L, min_demand = 1), check)
  hedgehog::forall(gen.demand_net(max_sites = 10L, max_demand = -1), check)
  hedgehog::forall(gen.demand_net(max_sites = 10L), check)
})

test_that("Solutions with mixed demands are consistent", {
  skip_if_not_installed("hedgehog")

  vehicles <- data.frame(n = c(2L, NA_integer_, NA_integer_), caps = c(35, 50, 80))

  hedgehog::forall(
    gen.demand_net(max_sites = 10L),
    function(demand_net) {
      res <-
        clarke_wright(
          demand_net$demand,
          demand_net$distances,
          vehicles,
          restrictions = data.frame(site = 0L, vehicle = 0L)
        )
      audit <- run_audit(res, demand_net$demand, demand_net$distances)

      # every site is visited exactly once
      expect_equal(sort(res$visits$site), seq_along(demand_net$demand) - 1)

      expect_equal(res$runs$distance, audit$distance)
      expect_equal(res$runs$max_load, audit$max_load)
      expect_true(all(audit$load_ok))
      expect_true(all(res$runs$max_load <= vehicles$caps[res$runs$vehicle + 1]))

      restricted_run <- res$visits[res$visits$site == 0, "run"]
      expect_false(0 %in% res$runs[res$runs$run == restricted_run, "vehicle"])
    }
  )
})

test_that("Identical input yields identical output, even with tied savings", {
  grid <- expand.grid(x = -4:4, y = -4:4)
  grid <- grid[grid$x != 0 | grid$y != 0, ]
  distances <- dist(rbind(data.frame(x = 0, y = 0), grid))
  demand <- rep(c(5, -5, 5), length.out = nrow(grid))
  vehicles <- data.frame(n = NA_integer_, caps = 33)

  res <- clarke_wright(demand, distances, vehicles)

  for (i in 1:10) {
    expect_equal(clarke_wright(demand, distances, vehicles), res)
  }
})

test_that("Demand exceeding vehicle capacity gets split, for either sign", {
  distances <- dist(data.frame(x = c(0, 1, 2, -1), y = c(0, 1, 0, 1)))
  vehicles <- data.frame(n = NA_integer_, caps = 30)

  # site 0 needs 3 full runs and 10 remain
  for (sign in c(1, -1)) {
    demand <- sign * c(100, 5, 5)
    res <- clarke_wright(demand, distances, vehicles)

    expect_equal(res$sites$demand, demand)
    expect_equal(sum(res$visits$site == 0), 4)
    expect_true(all(res$runs$max_load <= 30))
    expect_true(all(res$visits$load >= 0 & res$visits$load <= 30))

    # the 3 full runs serve only site 0
    full_runs <- res$visits[res$visits$run %in% res$runs$run[res$runs$max_load == 30], ]
    expect_equal(nrow(full_runs), 3)
    expect_equal(full_runs$site, rep(0, 3))
    expect_equal(full_runs$load, rep(if (sign > 0) 0 else 30, 3))

    # the remaining run serves the remainder and the other sites
    remaining_run <- res$runs[res$runs$max_load < 30, ]
    expect_equal(remaining_run$max_load, 20)
    expect_equal(
      sum(res$visits$run == remaining_run$run),
      3
    )
  }
})

test_that("Mixed run with a split site reports correct loads", {
  distances <- dist(data.frame(x = c(0, 1, 2, -1), y = c(0, 1, 0, 1)))

  # site 0 picks up 100: 3 full runs, and 10 are left to be combined
  # with the deliveries of 5 to each of site 1 and 2
  res <-
    clarke_wright(
      c(-100, 5, 5),
      distances,
      data.frame(n = NA_integer_, caps = 30)
    )

  mixed_run <- res$runs[res$runs$max_load < 30, ]
  visits <- res$visits[res$visits$run == mixed_run$run, ]
  visits <- visits[order(visits$order), ]
  site_demand <- c(-10, 5, 5)[visits$site + 1]

  expect_equal(visits$load, 10 - cumsum(site_demand))
  expect_equal(mixed_run$max_load, max(10, visits$load))
})

test_that("Fractional vehicle capacities are respected", {
  distances <- dist(data.frame(x = c(0, 1), y = c(0, 1)))

  split_loads <- function(demand, caps) {
    res <- clarke_wright(demand, distances, data.frame(n = NA_integer_, caps = caps))
    sort(res$runs$max_load)
  }

  expect_equal(split_loads(1, 0.5), c(0.5, 0.5))
  expect_equal(split_loads(-1, 0.5), c(0.5, 0.5))
  expect_equal(split_loads(1.4, 1.5), 1.4)
  expect_equal(split_loads(10, 1.5), c(1, rep(1.5, 6)))
  expect_equal(split_loads(10, 2.5), rep(2.5, 4))
  expect_equal(split_loads(33.2, 33.5), 33.2)
})

test_that("Running out of vehicles while splitting aborts, for either sign", {
  distances <- dist(data.frame(x = c(0, 1), y = c(0, 1)))

  for (demand in c(1, -1)) {
    expect_error(
      clarke_wright(demand, distances, data.frame(n = 1L, caps = 0.5)),
      "Not enough vehicles"
    )
  }
})

test_that("Very large demand splits do not exhaust the stack", {
  distances <- dist(data.frame(x = c(0, 1), y = c(0, 1)))

  res <- clarke_wright(1e6, distances, data.frame(n = NA_integer_, caps = 1))

  expect_equal(nrow(res$runs), 1e6)
})

test_that("Sites with zero demand are part of the solution", {
  distances <- dist(data.frame(x = c(0, 1, 2, -1), y = c(0, 1, 0, 1)))
  vehicles <- data.frame(n = NA_integer_, caps = 30)

  for (demand in list(c(0, 5, -5), c(0, 0, 0))) {
    res <- clarke_wright(demand, distances, vehicles)

    expect_equal(sort(res$visits$site), 0:2)
    expect_true(all(res$visits$load >= 0))
  }
})
