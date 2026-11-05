test_that("milkr_cost returns correct result", {
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

  expect_equal(
    milkr_cost(res),
    sum(1:10 * 2)
  )
})

test_that("milkr_saving returns correct result", {
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

  expect_equal(
    milkr_saving(res),
    0
  )

  expect_equal(
    milkr_saving(res, relative = TRUE),
    0
  )
})

test_that("milkr_saving accounts for round trips of split demand", {
  # a single site needs 3 round trips in any case: nothing to save
  res <- clarke_wright(
    15,
    dist(data.frame(pos_x = c(0, 0), pos_y = c(0, 1))),
    data.frame(n = NA_integer_, caps = 6)
  )

  expect_equal(milkr_saving(res), 0)
  expect_equal(milkr_saving(res, relative = TRUE), 0)

  # site 1 needs 3 round trips, of which the last one can be merged with site 2
  distances <- dist(data.frame(pos_x = c(0, 1, 2), pos_y = c(0, 0, 0)))
  res <- clarke_wright(
    c(15, 1),
    distances,
    data.frame(n = NA_integer_, caps = 6)
  )

  naive_cost <- 3 * (2 * 1) + 2 * 2
  expect_equal(milkr_cost(res), 2 * (2 * 1) + 2 * 2)
  expect_equal(milkr_saving(res), naive_cost - milkr_cost(res))
})
