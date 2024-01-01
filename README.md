
<!-- README.md is generated from README.Rmd. Please edit that file -->

# heumilkr

<!-- badges: start -->

[![R-CMD-check](https://github.com/lschneiderbauer/heumilkr/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/lschneiderbauer/heumilkr/actions/workflows/R-CMD-check.yaml)
[![Codecov test
coverage](https://codecov.io/gh/lschneiderbauer/heumilkr/branch/master/graph/badge.svg)](https://app.codecov.io/gh/lschneiderbauer/heumilkr?branch=master)
[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://lifecycle.r-lib.org/articles/stages.html#experimental)
[![CRAN
status](https://www.r-pkg.org/badges/version/heumilkr)](https://CRAN.R-project.org/package=heumilkr)
<!-- badges: end -->

heumilkr provides an implementation of the Clark-Wright algorithm to
find a quasi-optimal solution to the [Capacitated Vehicle Routing
Problem](https://en.wikipedia.org/wiki/Vehicle_routing_problem).

## Installation

You can install the development version of heumilkr from
[GitHub](https://github.com/) with:

``` r
# install.packages("devtools")
devtools::install_github("lschneiderbauer/heumilkr")
```

## Example

The following example generates random demands at random locations,
applies the Clark-Wright algorithm to generate quasi-optimal vehicle
runs and shows the resulting runs.

``` r
library(heumilkr)

set.seed(42)
demand <- runif(20, 5, 15)

pos <-
  data.frame(
    pos_x = c(0, runif(length(demand), -10, 10)),
    pos_y = c(0, runif(length(demand), -10, 10))
  )

res <-
  clark_wright(
    demand,
    dist(pos),
    data.frame(n = NA_integer_, caps = 33)
  )

print(res)
#>    run_id order vehicle_id
#> 1       0     0          0
#> 2       1     0          0
#> 3       0     2          0
#> 4       2     1          0
#> 5       1     1          0
#> 6       3     6          0
#> 7       3     8          0
#> 8       3     4          0
#> 9       3     7          0
#> 10      3     3          0
#> 11      3     1          0
#> 12      3     2          0
#> 13      4     0          0
#> 14      3     5          0
#> 15      4     2          0
#> 16      2     0          0
#> 17      4     3          0
#> 18      4     1          0
#> 19      0     1          0
#> 20      3     0          0
```

A plotting function (using ggplot) for the result is built in. The
individual runs are distinguished by color. The demanding site locations
are marked with round circles while the (single) supplying site is
depicted as a square.

``` r
plot(res)
```

<img src="man/figures/README-example_plot-1.png" width="100%" />

## Benchmarks

The benchmarks were taken on an Intel® Xeon® CPU E3-1231 v3 @ 3.40GHz
CPU.

The following graph shows the run time behavior as the number of sites
`n` increase. The curve exhibits near-cubic behavior in `n`. For
`n = 110` the performance is still relatively reasonable with a run time
of approximately `97 ms`.

<img src="man/figures/README-benchmark_runtime-1.png" width="100%" />
