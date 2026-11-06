# heumilkr (development version)

## Breaking changes

-   `clarke_wright()`: Return value changed: instead of returning a single `data.frame()` a list of three `data.frame()`s is returned now that makes it easier to interpret the result.

-   `cvrplib_ls()` now returns a `data.frame()` instead of a simple vector.

-   "`cvrplib_instance`" objects, including the bundled data sets: the field `optimum` is renamed to `best_known_cost`, since CVRPLIB provides best known solutions that are not necessarily proven optima.

-   `cvrplib_download()` now raises an error if a solution file exists but can not be downloaded, instead of silently setting the value to `NA`.

## New features / improvements

-   `cvrplib_download()` now accepts a vector of qualifiers and returns a named list of instances in that case. The list of available instances is only requested once. A single qualifier still returns the instance itself.

-   `clarke_wright()` now supports negative demands: sites with negative demand are treated as "backhaul" points, i.e. they deliver goods back to the depot. Sites with positive and negative demands can be visited in any order within a run, as long as the load stays within the vehicle capacity at every point of the run. Negative demands that exceed vehicle capacities are split across multiple runs, like positive demands.

-   `clarke_wright()` has gained a large runtime speedup compared to the previous version: for 500 sites the run time drops from about 6 seconds to about 0.01 seconds in our benchmarks. The actual speedup depends on input characteristics and grows with the number of sites.

-   `clarke_wright()`: Results are now deterministic, also in case of tied savings. Run identifiers follow the order of run creation.


## Bugfixes

-   `clarke_wright()`: Fractional vehicle capacities are not truncated to integers anymore. Previously this caused unnecessary splits of demand, and capacities below 1 never terminated.

-   `clarke_wright()`: Fix a crash with sites of zero demand.

-   `clarke_wright()`: The `load` of visits now always refers to the load after the visit, also for sites whose demand is split across multiple runs.

-   `plot()`, `autoplot()`: Fix errors and misplaced visits for solutions with sites whose demand is split across multiple runs.

-   `milkr_saving()`: The naive cost now includes the additional round trips of sites whose demand is split across multiple runs. Previously the saving could become negative.

-   `cvrplib_download()`, `clarke_wright_cvrplib()`: Instances defined by an explicit distance matrix (including `E-n13-k4` and `E-n31-k7` in `cvrplib_E`) got corrupted distances, since coordinates were derived from them incorrectly. The matrix is now kept in the new field `distances` and used by the solver, and the coordinates in `pos` are `NA` for those instances.

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
