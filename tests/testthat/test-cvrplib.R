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

  inst <- cvrplib_download("A-n80-k10")

  expect_s3_class(
    inst,
    "cvrplib_instance"
  )

  expect_equal(
    inst,
    cvrplib_A[[27]]
  )
})

test_that("CVRP downloader handles solution files correctly", {
  instance_file <- tempfile(fileext = ".vrp")
  on.exit(unlink(instance_file))
  writeLines(
    c(
      "NAME : test-n2",
      "COMMENT : test",
      "TYPE : CVRP",
      "DIMENSION : 2",
      "EDGE_WEIGHT_TYPE : EUC_2D",
      "CAPACITY : 10",
      "NODE_COORD_SECTION",
      "1 0 0",
      "2 1 1",
      "DEMAND_SECTION",
      "1 0",
      "2 5",
      "DEPOT_SECTION",
      "1",
      "-1",
      "EOF"
    ),
    instance_file
  )

  mock_ls <- function(solution_url) {
    function(cvrplib_url) {
      data.frame(
        name = "test-n2",
        instance_url = paste0("file://", instance_file),
        solution_url = solution_url
      )
    }
  }

  # no solution file available
  local_mocked_bindings(cvrplib_ls = mock_ls(NA_character_))
  expect_true(is.na(cvrplib_download("test-n2")$best_known_cost))

  # a solution file that can not be downloaded is an error
  local_mocked_bindings(
    cvrplib_ls = mock_ls(paste0("file://", instance_file, ".missing"))
  )
  suppressWarnings(expect_error(cvrplib_download("test-n2")))

  # a solution file is parsed
  solution_file <- tempfile(fileext = ".sol")
  on.exit(unlink(solution_file), add = TRUE)
  writeLines(c("Route #1: 1", "Cost 2.5"), solution_file)
  local_mocked_bindings(
    cvrplib_ls = mock_ls(paste0("file://", solution_file))
  )
  expect_equal(cvrplib_download("test-n2")$best_known_cost, 2.5)
})
