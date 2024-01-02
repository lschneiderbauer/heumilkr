test_that("autoplot does not err", {
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

  pl <- autoplot(res)

  expect_s3_class(pl, "ggplot")
})
