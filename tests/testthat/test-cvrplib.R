test_that("Applying clarke_wright to cvrplib data results in a valid solution object.", {
  expect_s3_class(
    clarke_wright_cvrplib(
      cvrplib_A[[1]]
    ),
    "heumilkr_solution"
  )
})

test_that("CVRP downloader yields correct result", {
  skip_if_offline()
  skip_on_cran()

  inst <- cvrplib_download("A/A-n80-k10")

  expect_s3_class(
    inst,
    "cvrplib_instance"
  )

  expect_equal(
    inst,
    cvrplib_A[[27]]
  )
})
