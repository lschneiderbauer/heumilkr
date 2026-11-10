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
  skip_on_ci()

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

test_that("CVRP downloader keeps explicit distance matrices", {
  instance_file <- tempfile(fileext = ".vrp")
  on.exit(unlink(instance_file))

  # distances: depot-1: 1, depot-2: 2, 1-2: 4
  # (the triangle inequality does not hold on purpose)
  formats <- list(
    LOWER_ROW = c("1", "2 4"),
    LOWER_DIAG_ROW = c("0", "1 0", "2 4 0"),
    FULL_MATRIX = c("0 1 2", "1 0 4", "2 4 0")
  )

  local_mocked_bindings(
    cvrplib_ls = function(cvrplib_url) {
      data.frame(
        name = "test-n3",
        instance_url = paste0("file://", instance_file),
        solution_url = NA_character_
      )
    }
  )

  for (format in names(formats)) {
    writeLines(
      c(
        "NAME : test-n3",
        "COMMENT : test",
        "TYPE : CVRP",
        "DIMENSION : 3",
        "EDGE_WEIGHT_TYPE : EXPLICIT",
        paste0("EDGE_WEIGHT_FORMAT : ", format),
        "CAPACITY : 10",
        "EDGE_WEIGHT_SECTION",
        formats[[format]],
        # real instances can provide coordinates next to the explicit distances
        "NODE_COORD_SECTION",
        "1 0 0",
        "2 1 0",
        "3 2 0",
        "DEMAND_SECTION",
        "1 0",
        "2 5",
        "3 5",
        "DEPOT_SECTION",
        "1",
        "-1",
        "EOF"
      ),
      instance_file
    )

    expect_no_warning(inst <- cvrplib_download("test-n3"))

    expect_equal(as.numeric(inst$distances), c(1, 2, 4))
    expect_true(all(is.na(inst$pos$x)))
    expect_true(all(is.na(inst$pos$y)))

    res <- clarke_wright_cvrplib(inst)
    expect_equal(as.numeric(attr(res, "distances")), c(1, 2, 4))
  }
})

test_that("Bundled instances with explicit distances reproduce the published cost", {
  route_cost <- function(inst, routes) {
    d <- as.matrix(inst$distances)

    sum(
      vapply(
        routes,
        function(route) {
          # the solution files number the sites without the depot
          path <- c(1, route + 1, 1)
          sum(d[cbind(path[-length(path)], path[-1])])
        },
        FUN.VALUE = 1
      )
    )
  }

  inst <- Filter(function(x) x$name == "E-n13-k4", cvrplib_E)[[1]]

  expect_equal(
    route_cost(inst, list(1, c(8, 5, 3), c(9, 12, 10, 6), c(11, 4, 7, 2))),
    inst$best_known_cost
  )
})

test_that("CVRP downloader handles one or several qualifiers", {
  write_instance <- function(name) {
    file <- tempfile(fileext = ".vrp")
    writeLines(
      c(
        paste0("NAME : ", name),
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
      file
    )
    file
  }

  files <- c(a = write_instance("a"), b = write_instance("b"))
  on.exit(unlink(files))

  n_ls_calls <- 0
  local_mocked_bindings(
    cvrplib_ls = function(cvrplib_url) {
      n_ls_calls <<- n_ls_calls + 1
      data.frame(
        name = names(files),
        instance_url = paste0("file://", files),
        solution_url = NA_character_
      )
    }
  )

  # a single qualifier yields the instance itself
  single <- cvrplib_download("a")
  expect_s3_class(single, "cvrplib_instance")
  expect_equal(single$name, "a")

  # several qualifiers yield a named list in the requested order,
  # and the available instances are only requested once
  n_ls_calls <- 0
  several <- cvrplib_download(c("b", "a"))
  expect_equal(n_ls_calls, 1)
  expect_type(several, "list")
  expect_named(several, c("b", "a"))
  expect_equal(several$b$name, "b")
  expect_equal(several$a, single)

  expect_error(cvrplib_download(c("a", "nope")), "nope")
  expect_error(cvrplib_download(character()))
})

test_that("Solution quality on the bundled CVRPLIB sets does not get worse.", {
  # Total cost of the solutions per set. Better solutions pass. If the cost
  # changes on purpose, replace the values with the output of
  # vapply(sets, set_cost, numeric(1)).
  baseline <- c(
    A = 29484.783712,
    B = 23187.748305,
    E = 9339.007433,
    F = 2213.570932,
    Tai = 24695.303004
  )

  sets <- list(
    A = cvrplib_A,
    B = cvrplib_B,
    E = cvrplib_E,
    F = cvrplib_F,
    Tai = cvrplib_Tai
  )
  set_cost <- function(set) {
    sum(vapply(
      set,
      function(inst) milkr_cost(clarke_wright_cvrplib(inst)),
      numeric(1)
    ))
  }

  cost <- vapply(sets, set_cost, numeric(1))

  for (set in names(baseline)) {
    expect_lte(cost[[set]], baseline[[set]] * (1 + 1e-6), label = set)
  }
})
