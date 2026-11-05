## code to prepare `cvrplib` dataset goes here
library(purrr)
library(usethis)

instances <- cvrplib_ls()$name

# TAI - solution files contain optimal solution according to website
selected_instances <-
  list(
    # Rochat and Taillard, 1995
    Tai = setdiff(
      instances[grepl("^tai", instances)],
      "tai385"
    ),
    # Augerat, 1995
    A = instances[grepl("^A-", instances)],
    # Augerat, 1995
    B = instances[grepl("^B-", instances)],
    # Christofides and Eilon, 1969
    E = instances[grepl("^E-", instances)],
    # Fisher, 1994
    F = instances[grepl("^F-", instances)]
  )

data <-
  map(
    set_names(names(selected_instances)),
    function(group) {
      map(
        set_names(selected_instances[[group]]),
        cvrplib_download
      )
    },
    .progress = TRUE
  )

walk2(data, names(data), function(obj, name) {
  name <- paste0("cvrplib_", name)
  assign(name, obj)
  do.call("use_data", list(as.name(name), overwrite = TRUE))
})
