test_that("Applying clarke_wright to cvrplib data results in a valid solution object.", {
  expect_s3_class(
    clarke_wright_cvrplib(
      cvrplib_A[[1]]
    ),
    "heumilkr_solution"
  )
})

test_that("CVRP downloader yields a result", {
  skip_if_offline()

  expect_s3_class(
    cvrplib_download("A/A-n80-k10"),
    "cvrplib_instance"
  )
})
