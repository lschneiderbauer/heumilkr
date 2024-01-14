library(heumilkr)
library(purrr)

sha <- system("git rev-parse HEAD", intern=TRUE)

# TAI - solution files contain optimal solution according to website
selected_instances <-
  list(
    tai = cvrplib_Tai,
    A = cvrplib_A,
    B = cvrplib_B,
    E = cvrplib_E,
    F = cvrplib_F
  )

result <-
  map(
    names(selected_instances),
    function(group) {
      map(
        selected_instances[[group]],
        function(inst) {
          clarke_wright_perf_xi <- cvrplib_clarke_wright_perf(inst)
          clarke_wright_perf_rho <- cvrplib_clarke_wright_perf2(inst)

          data.frame(
            name = inst$name,
            group,
            n_site = inst$dimension - 1,
            clarke_wright_perf_xi,
            clarke_wright_perf_rho
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
