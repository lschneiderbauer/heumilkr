# Checks of the solver input that concern R itself (classes, columns, types),
# and conversion to what the C++ core expects. The core checks the values
# itself (see src/input_error.h), and abort_input_error() reports its findings.
prepare_solver_input <- function(
  demand,
  distances,
  vehicles,
  restrictions,
  call = rlang::caller_env()
) {
  if (!is.numeric(demand)) {
    cli::cli_abort(
      "{.arg demand} must be a numeric vector, not {.obj_type_friendly {demand}}.",
      call = call
    )
  }

  if (!inherits(distances, "dist")) {
    cli::cli_abort(
      c(
        "{.arg distances} must be a {.cls dist} object, not {.obj_type_friendly {distances}}.",
        i = "Create one with {.fn stats::dist} or {.fn stats::as.dist}."
      ),
      call = call
    )
  }

  n_locations <- attr(distances, "Size")
  if (n_locations != length(demand) + 1) {
    cli::cli_abort(
      c(
        "{.arg distances} must describe {length(demand) + 1} locations: the depot and the {length(demand)} site{?s} of {.arg demand}.",
        x = "It describes {n_locations} location{?s}."
      ),
      call = call
    )
  }

  check_columns(vehicles, c("n", "caps"), call)
  check_columns(restrictions, c("vehicle", "site"), call)

  caps <- vehicles$caps
  # missing capacities are reported by the core, like other invalid ones
  if (is.logical(caps) && all(is.na(caps))) {
    caps <- as.numeric(caps)
  }
  if (!is.numeric(caps)) {
    cli::cli_abort(
      "{.code vehicles$caps} must be numeric, not {.obj_type_friendly {caps}}.",
      call = call
    )
  }

  n <- as_whole_numbers(vehicles$n, "vehicles$n", call)
  # an unlimited number of vehicles
  n[is.na(n)] <- .Machine$integer.max

  list(
    demand = as.numeric(demand),
    distances = distances,
    n = n,
    caps = as.numeric(caps),
    restr_sites = as_whole_numbers(restrictions$site, "restrictions$site", call),
    restr_vehicles = as_whole_numbers(restrictions$vehicle, "restrictions$vehicle", call)
  )
}

check_columns <- function(x, columns, call) {
  arg <- deparse(substitute(x))

  if (!is.data.frame(x)) {
    cli::cli_abort(
      "{.arg {arg}} must be a data frame, not {.obj_type_friendly {x}}.",
      call = call
    )
  }

  missing <- setdiff(columns, colnames(x))
  if (length(missing) > 0) {
    cli::cli_abort(
      c(
        "{.arg {arg}} must have the {cli::qty(columns)}column{?s} {.code {columns}}.",
        x = "{cli::qty(missing)}Column{?s} {.code {missing}} {?is/are} missing."
      ),
      call = call
    )
  }
}

# Converts whole numbers (also when stored as doubles) to integers, as the C++
# core expects. `NA`s are kept.
as_whole_numbers <- function(x, arg, call) {
  if (is.integer(x)) {
    return(x)
  }

  if (is.logical(x) && all(is.na(x))) {
    return(as.integer(x))
  }

  if (!is.numeric(x)) {
    cli::cli_abort(
      "{.code {arg}} must contain whole numbers, not {.obj_type_friendly {x}}.",
      call = call
    )
  }

  invalid <- which(
    !is.na(x) & (x != round(x) | abs(x) > .Machine$integer.max)
  )
  if (length(invalid) > 0) {
    i <- invalid[[1]]
    cli::cli_abort(
      c(
        "{.code {arg}} must contain whole numbers.",
        x = "{.code {arg}[{i}]} is {.val {x[[i]]}}."
      ),
      call = call
    )
  }

  as.integer(x)
}

# Runs one of the C++ solver entry points, which return list(error = ...)
# instead of a result when the core rejects the input.
call_solver <- function(fun, input, call = rlang::caller_env()) {
  res <- fun(
    input$demand,
    input$distances,
    input$n,
    input$caps,
    input$restr_sites,
    input$restr_vehicles
  )

  if (!is.null(res$error)) {
    abort_input_error(res$error, input, call)
  }

  res
}

# Reports an error found by the C++ core in the terms of the R user. The core
# counts from 0, R positions start at 1. Site and vehicle type indices are part
# of the package interface and start at 0 in R as well.
abort_input_error <- function(error, input, call) {
  message <- input_error_messages[[error$code]]
  if (is.null(message)) {
    cli::cli_abort(
      "Unknown input error {.val {error$code}}.",
      .internal = TRUE,
      call = call
    )
  }

  message <- message(error, input)

  cli::cli_abort(
    message$text,
    .envir = message$envir,
    class = c(paste0("heumilkr_error_", error$code), "heumilkr_error_input"),
    .internal = error$code %in% internal_input_errors,
    call = call
  )
}

# Mismatches that prepare_solver_input() rules out already
internal_input_errors <- c(
  "wrong_distance_count",
  "vehicle_length_mismatch",
  "restriction_length_mismatch"
)

# A message for cli::cli_abort(), together with the environment its glue
# expressions refer to
input_message <- function(text, envir = parent.frame()) {
  list(text = text, envir = envir)
}

# One message per error code of the core
input_error_messages <- list(
  non_finite_demand = function(error, input) {
    i <- error$index + 1
    value <- input$demand[[i]]
    input_message(c(
      "{.arg demand} must not contain missing or infinite values.",
      x = "{.code demand[{i}]} is {.val {value}}."
    ))
  },
  wrong_distance_count = function(error, input) {
    input_message("The C++ core expected {error$other_index} distances, but got {error$index}.")
  },
  non_finite_distance = function(error, input) {
    locations <- c(error$index, error$other_index) + 1
    labels <- attr(input$distances, "Labels")
    if (!is.null(labels)) {
      locations <- labels[locations]
    }
    value <- error$value
    input_message(c(
      "{.arg distances} must not contain missing or infinite values.",
      x = "The distance between locations {.val {locations[[1]]}} and {.val {locations[[2]]}} is {.val {value}}."
    ))
  },
  no_vehicle_types = function(error, input) {
    input_message("{.arg vehicles} must have at least one row.")
  },
  vehicle_length_mismatch = function(error, input) {
    input_message("The C++ core got {error$index} vehicle capacities, but {error$other_index} vehicle counts.")
  },
  negative_vehicle_count = function(error, input) {
    i <- error$index + 1
    value <- input$n[[i]]
    input_message(c(
      "{.code vehicles$n} must not be negative.",
      x = "{.code vehicles$n[{i}]} is {.val {value}}.",
      i = "Use {.code NA} for an unlimited number of vehicles."
    ))
  },
  invalid_vehicle_capacity = function(error, input) {
    i <- error$index + 1
    value <- input$caps[[i]]
    input_message(c(
      "{.code vehicles$caps} must be positive and finite.",
      x = "{.code vehicles$caps[{i}]} is {.val {value}}."
    ))
  },
  restriction_length_mismatch = function(error, input) {
    input_message("The C++ core got {error$index} restricted vehicles, but {error$other_index} restricted sites.")
  },
  unknown_restriction_site = function(error, input) {
    i <- error$index + 1
    value <- input$restr_sites[[i]]
    numbering <- describe_numbering(
      length(input$demand),
      "site",
      "in the order of {.arg demand}"
    )
    input_message(c(
      "{.code restrictions$site} must refer to sites of {.arg demand}.",
      x = "{.code restrictions$site[{i}]} is {.val {value}}.",
      i = numbering
    ))
  },
  unknown_restriction_vehicle = function(error, input) {
    i <- error$index + 1
    value <- input$restr_vehicles[[i]]
    numbering <- describe_numbering(
      length(input$caps),
      "vehicle type",
      "in the order of the rows of {.arg vehicles}"
    )
    input_message(c(
      "{.code restrictions$vehicle} must refer to rows of {.arg vehicles}.",
      x = "{.code restrictions$vehicle[{i}]} is {.val {value}}.",
      i = numbering
    ))
  },
  insufficient_vehicles = function(error, input) {
    site <- error$index
    i <- site + 1
    input_message(c(
      "Not enough vehicles available to start with one run per site.",
      x = "Ran out of vehicles at site {site} ({.code demand[{i}]}).",
      i = "The solver merges runs only afterwards, so it can need more vehicles at the start than in the final solution.",
      i = "A vehicle type with an unlimited number ({.code n = NA}) and without restrictions avoids this."
    ))
  }
)

# e.g. "Sites are numbered from 0 to 4, in the order of `demand`.", for
# indices that start at 0. Returns cli markup.
describe_numbering <- function(n, what, order) {
  if (n == 0) {
    paste0("There are no ", what, "s.")
  } else if (n == 1) {
    paste0("The only ", what, " is 0.")
  } else {
    paste0(
      toupper(substr(what, 1, 1)), substring(what, 2), "s are numbered from 0 to ",
      n - 1, ", ", order, "."
    )
  }
}
