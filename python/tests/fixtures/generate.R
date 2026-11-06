# Generates r_results.json: solutions of the R package, which the Python tests
# compare against, so that the two packages can not silently drift apart.
#
# Run this from the root of the repository after an intended change of solver
# results: Rscript python/tests/fixtures/generate.R

devtools::load_all(quiet = TRUE)

# Inputs are rounded, so they keep their exact value when stored as JSON.
scenario <- function(name, demand, positions, vehicles, restrictions = NULL) {
  demand <- round(demand, 6)
  distances <- round(dist(positions), 6)

  solution <-
    if (is.null(restrictions)) {
      clarke_wright(demand, distances, vehicles)
    } else {
      clarke_wright(demand, distances, vehicles, restrictions)
    }

  list(
    name = name,
    demand = demand,
    distances = as.numeric(distances),
    vehicles = list(
      capacity = vehicles$caps,
      # JSON has no NA in integer vectors that survives reliably, -1 = unlimited
      count = ifelse(is.na(vehicles$n), -1L, vehicles$n)
    ),
    restrictions = if (is.null(restrictions)) {
      list(site = integer(), vehicle = integer())
    } else {
      as.list(restrictions)
    },
    runs = as.list(solution$runs),
    visits = as.list(solution$visits),
    cost = milkr_cost(solution),
    saving = milkr_saving(solution)
  )
}

random_positions <- function(n) {
  data.frame(x = c(0, runif(n, -10, 10)), y = c(0, runif(n, -10, 10)))
}

set.seed(42)

scenarios <- list(
  scenario(
    "deliveries with two vehicle types",
    runif(20, 5, 15),
    random_positions(20),
    data.frame(n = c(NA_integer_, 2L), caps = c(33, 44))
  ),
  scenario(
    "deliveries and pick-ups with restrictions",
    runif(30, -15, 15),
    random_positions(30),
    data.frame(n = c(3L, NA_integer_), caps = c(35, 50)),
    data.frame(site = c(0L, 4L), vehicle = c(0L, 0L))
  ),
  scenario(
    "split demand",
    c(100, -70, 5, 5, -5),
    random_positions(5),
    data.frame(n = NA_integer_, caps = 30)
  )
)

jsonlite::write_json(
  scenarios,
  "python/tests/fixtures/r_results.json",
  auto_unbox = TRUE,
  digits = NA,
  pretty = TRUE
)
