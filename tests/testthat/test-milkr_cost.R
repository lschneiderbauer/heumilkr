test_that("milkr_cost returns correct result", {
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
    demand, dist(pos),
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
