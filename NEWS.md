# heumilkr (development version)

## Breaking changes

-   `clarke_wright()`: The default argument of `restrictions` changed from `NULL` to an empty data frame. Passing `restrictions = NULL` now fails.

## Bugfixes

-   `clarke_wright()`:

    -   Fixed a bug where vehicle restrictions would not be applied correctly if two tours were linked whose end points did not have vehicle restrictions maintained, but the tours included other edges with vehicle restrictions.

    -   Fixed integer overflow bugs which caused the R session to crash in certain circumstances.

# heumilkr 0.2.0

## Enhancements

-   Add the `cvrp_*()` function family for handling [CVRPLIB](http://vrp.atd-lab.inf.puc-rio.br/) data.

-   New vignette with performance benchmarks: `vignette("clarke_wright_performance")`.

## Bugfixes

-   Fix `clarke_wright()` crashing R when passing `NA` values to the `distances` argument: it stops gracefully instead.

# heumilkr 0.1.0

-   Initial CRAN submission.
