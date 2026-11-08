positions <- data.frame(x = c(0, 1, -1, -2), y = c(0, 1, 1, 2))
distances <- dist(positions)
unlimited <- data.frame(n = NA, caps = 6)

test_that("Each error of the C++ core has an R message", {
  expect_setequal(names(input_error_messages), cpp_input_error_names())
})

test_that("Errors of the C++ core are reported in R terms", {
  missing_distance <- as.matrix(distances)
  missing_distance[2, 4] <- missing_distance[4, 2] <- NA

  expect_snapshot(error = TRUE, {
    clarke_wright(c(1, NA, 3), distances, unlimited)
    clarke_wright(c(1, 2, Inf), distances, unlimited)
    clarke_wright(1:3, as.dist(missing_distance), unlimited)
    clarke_wright(1:3, distances, data.frame(n = integer(), caps = numeric()))
    clarke_wright(1:3, distances, data.frame(n = c(NA, -2L), caps = 6))
    clarke_wright(1:3, distances, data.frame(n = NA, caps = c(6, 0)))
    clarke_wright(1:3, distances, data.frame(n = NA, caps = NA))
    clarke_wright(
      1:3, distances, unlimited,
      data.frame(vehicle = 0, site = c(0, 3))
    )
    clarke_wright(
      1:3, distances, unlimited,
      data.frame(vehicle = 1, site = 0)
    )
    clarke_wright(c(15, 3, 1), distances, data.frame(n = 2L, caps = 6))
  })
})

test_that("Errors of the C++ core have classes", {
  expect_error(
    clarke_wright(c(1, NA, 3), distances, unlimited),
    class = "heumilkr_error_non_finite_demand"
  )
  expect_error(
    clarke_wright(c(15, 3, 1), distances, data.frame(n = 2L, caps = 6)),
    class = "heumilkr_error_insufficient_vehicles"
  )
  expect_error(
    clarke_wright(1:3, distances, data.frame(n = NA, caps = -1)),
    class = "heumilkr_error_input"
  )
})

test_that("Inputs of the wrong type are reported", {
  expect_snapshot(error = TRUE, {
    clarke_wright("a", distances, unlimited)
    clarke_wright(1:3, as.matrix(distances), unlimited)
    clarke_wright(1:2, distances, unlimited)
    clarke_wright(1:3, distances, list(n = NA, caps = 6))
    clarke_wright(1:3, distances, data.frame(caps = 6))
    clarke_wright(1:3, distances, data.frame(n = NA, caps = "6"))
    clarke_wright(1:3, distances, data.frame(n = 1.5, caps = 6))
    clarke_wright(1:3, distances, unlimited, data.frame(site = 0))
    clarke_wright(1:3, distances, unlimited, data.frame(site = "a", vehicle = 0))
  })
})

test_that("Whole numbers stored as doubles are accepted", {
  expect_equal(
    clarke_wright(
      1:3, distances,
      data.frame(n = c(1, NA), caps = c(3, 6)),
      data.frame(site = 2, vehicle = 0)
    ),
    clarke_wright(
      1:3, distances,
      data.frame(n = c(1L, NA_integer_), caps = c(3, 6)),
      data.frame(site = 2L, vehicle = 0L)
    )
  )
})

test_that("The stepwise solver reports errors in the same way", {
  expect_error(
    clarke_wright_stepwise(c(1, NA, 3), distances, unlimited),
    class = "heumilkr_error_non_finite_demand"
  )
})
