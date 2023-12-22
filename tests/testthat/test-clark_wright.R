test_that("multiplication works", {
  demand <- c(12, 3, 14)

  distances <-
    dist(
      data.frame(
        pos_x = c(0, sample(10, 3, replace = TRUE)),
        pos_y = c(0, sample(10, 3, replace = TRUE))
      ),
      method = "euclidean"
    )

  res <- clark_wright(demand, distances)

  expect_equal(length(res), 1)
  expect_equal(
    sort(res[[1]]),
    c(0, 1, 2)
  )
})
