# heumilkr (development version)

## Breaking changes

-   `clarke_wright()`: Return value changed: instead of returning a single `data.frame()` a list of three `data.frame()`s is returned now that makes it easier to interpret the result.

-   `cvrplib_ls()` now returns a `data.frame()` instead of a simple vector.

## New features / improvements

-   `clarke_wright()` now supports negative demands: sites with negative demand are treated as "backhaul" points, i.e. they deliver goods back to the depot. By design they can only be visited after all positive-demand sites have been served.

-   `clarke_wright()` has gained runtime speedup compared to the previous version by roughly a factor of 3 (the actual speedup depends on input characteristics).


## Bugfixes

-   `cvrplib_ls()`, `cvrplib_download()`: Upstream website has changed. Fix malfunctioning functions by adjusting webscraping accordingly.

# heumilkr 0.3.0

## Breaking changes

-   `clarke_wright()`: The default argument of `restrictions` is changed from `NULL` to an empty data frame. Passing `restrictions = NULL` now fails.

## Minor improvements and fixes

-   `clarke_wright()`:

    -   Vehicle restrictions are now correctly applied if two tours are linked whose end points do not have vehicle restrictions maintained, but the tours include other edges with vehicle restrictions.

    -   R session does not crash anymore due to integer overflow issues in certain circumstances.

-   `plot()`/`autoplot()`:

    -   Plotting of a milk run result with a single site now also works.

    -   The functions do have reference pages now.

# heumilkr 0.2.0

## New features

-   Add the `cvrp_*()` function family for handling [CVRPLIB](http://vrp.atd-lab.inf.puc-rio.br/) data.

-   New vignette with performance benchmarks: `vignette("clarke_wright_performance")`.

## Minor improvements and fixes

-   `clarke_wright()` does not crash R anymore when passing `NA` values to the `distances` argument: it stops gracefully instead.

# heumilkr 0.1.0

-   Initial CRAN submission.
