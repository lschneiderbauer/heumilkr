test_that("multiplication works", {
  demand <- c(12, 3, 14)

  pos <-
    data.frame(
      pos_x = c(0, runif(length(demand), -10, 10)),
      pos_y = c(0, runif(length(demand), -10, 10))
    )

  distances <- dist(pos, method = "euclidean")

  res <- clark_wright(demand, distances, data.frame(n=1L, caps = 33))

  expect_equal(length(res), 1)
  expect_equal(
    sort(res[[1]]),
    c(0, 1, 2)
  )
})
