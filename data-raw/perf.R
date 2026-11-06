# Run this script manually from the package root with the current version of the
# package installed. Do not use devtools::load_all(): it compiles without
# optimizations, which invalidates the runtime measurements. The vignette only reads
# the resulting vignettes/perf.rds, so building the package does not require any of
# this.
#
# Besides the bundled instances this uses instance sets from CVRPLIB. To go easy on
# the website they are only downloaded once, as one archive per set, and are kept in
# the (git-ignored) folder data-raw/instances. Extracting the archives
# requires the command line tool 7z.
library(heumilkr)
library(purrr)
library(bench)

sha <- system("git rev-parse HEAD", intern = TRUE)

instance_dir <- "./data-raw/instances"

# returns the instances of a CVRPLIB set
read_cvrplib_set <- function(
  set_id,
  set_name,
  download_url = "https://galgos.inf.puc-rio.br/cvrplib/en/download"
) {
  set_dir <- file.path(instance_dir, set_name)

  if (!dir.exists(set_dir)) {
    dir.create(set_dir, recursive = TRUE)

    for (kind in c("instance-set", "instance-set-bks")) {
      archive <- tempfile(fileext = ".7z")
      download.file(
        paste0(download_url, "/", kind, "/", set_id),
        archive,
        mode = "wb"
      )
      status <- system2(
        "7z",
        c("x", "-y", paste0("-o", set_dir), archive),
        stdout = FALSE
      )
      stopifnot(status == 0)
      unlink(archive)
    }
  }

  instance_files <- list.files(set_dir, pattern = "[.]vrp$", full.names = TRUE)

  map(
    instance_files,
    function(file) {
      file <- normalizePath(file)

      heumilkr:::cvrplib_read_instance(
        paste0("file://", file),
        paste0("file://", sub("[.]vrp$", ".sol", file))
      )
    }
  )
}

dimacs <- read_cvrplib_set(19, "DIMACS")
has_prefix <- function(instances, prefix) {
  keep(instances, function(inst) startsWith(inst$name, prefix))
}

selected_instances <-
  list(
    tai = cvrplib_Tai,
    A = cvrplib_A,
    B = cvrplib_B,
    E = cvrplib_E,
    F = cvrplib_F,
    X = read_cvrplib_set(17, "X"),
    # extracted from real problems
    Loggi = has_prefix(dimacs, "Loggi"),
    ORTEC = has_prefix(dimacs, "ORTEC")
  )

result <-
  map(
    names(selected_instances),
    function(group) {
      map(
        selected_instances[[group]],
        function(inst) {
          clarke_wright_perf_xi <- heumilkr:::cvrplib_clarke_wright_perf(inst)
          clarke_wright_perf_rho <- heumilkr:::cvrplib_clarke_wright_perf2(inst)

          runtime <- as.numeric(
            mark(
              clarke_wright_cvrplib(inst),
              min_time = 0.1,
              min_iterations = 3
            )$median
          )

          data.frame(
            name = inst$name,
            group,
            n_site = inst$dimension - 1,
            clarke_wright_perf_xi,
            clarke_wright_perf_rho,
            runtime
          )
        }
      )
    },
    .progress = TRUE
  ) |>
  list_flatten() |>
  list_rbind()

filename <- paste0(format(Sys.time(), "%Y%m%d"), "-", sha, ".rds")

writeLines(filename, "./benchmark/performance/last_result.txt")
saveRDS(result, paste0("./benchmark/performance/", filename))

# write also for vignettes
saveRDS(result, "./vignettes/perf.rds")
