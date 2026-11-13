# Changelog

## heumilkr 0.4.0

CRAN release: 2026-11-13

### Breaking changes

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md):
  Return value changed: instead of returning a single
  [`data.frame()`](https://rdrr.io/r/base/data.frame.html) a list of
  three [`data.frame()`](https://rdrr.io/r/base/data.frame.html)s is
  returned now that makes it easier to interpret the result.

- [`cvrplib_ls()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_ls.md)
  now returns a [`data.frame()`](https://rdrr.io/r/base/data.frame.html)
  instead of a simple vector.

- “`cvrplib_instance`” objects, including the bundled data sets: the
  field `optimum` is renamed to `best_known_cost`, since CVRPLIB
  provides best known solutions that are not necessarily proven optima.

- [`cvrplib_download()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_download.md)
  now raises an error if a solution file exists but can not be
  downloaded, instead of silently setting the value to `NA`.

### New features / improvements

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  has more informative error messages: they name the offending argument
  and element, e.g. `` `vehicles$caps[2]` is 0 ``, and have the classes
  `heumilkr_error_input` and `heumilkr_error_<problem>`
  (e.g. `heumilkr_error_insufficient_vehicles`) to be caught
  specifically. Infinite values in `demand` and `distances` are rejected
  now (they could make the solver loop forever), as well as negative
  vehicle counts and restrictions that refer to sites or vehicle types
  that do not exist. Vehicle counts and restriction indices can now also
  be whole numbers stored as doubles,
  e.g. `data.frame(site = 1, vehicle = 0)`.

- [`cvrplib_download()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_download.md)
  now accepts a vector of qualifiers and returns a named list of
  instances in that case. The list of available instances is only
  requested once. A single qualifier still returns the instance itself.

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  now supports negative demands: sites with negative demand are treated
  as “backhaul” points, i.e. they deliver goods back to the depot. Sites
  with positive and negative demands can be visited in any order within
  a run, as long as the load stays within the vehicle capacity at every
  point of the run. Negative demands that exceed vehicle capacities are
  split across multiple runs, like positive demands.

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  has gained a large runtime speedup compared to the previous version:
  for 500 sites the run time drops from about 6 seconds to about 0.007
  seconds in our benchmarks. The actual speedup depends on input
  characteristics and grows with the number of sites. In return, large
  problems need more memory: the savings between sites now take about
  `8 * n^2` bytes for `n` sites instead of half of that.

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md):
  Results are now deterministic, also in case of tied savings. Run
  identifiers follow the order of run creation.

### Bugfixes

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md):
  Fractional vehicle capacities are not truncated to integers anymore.
  Previously this caused unnecessary splits of demand, and capacities
  below 1 never terminated.

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md):
  Fix a crash with sites of zero demand.

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md):
  The `load` of visits now always refers to the load after the visit,
  also for sites whose demand is split across multiple runs.

- [`plot()`](https://rdrr.io/r/graphics/plot.default.html),
  `autoplot()`: Fix errors and misplaced visits for solutions with sites
  whose demand is split across multiple runs.

- [`milkr_saving()`](https://lschneiderbauer.github.io/heumilkr/reference/milkr_saving.md):
  The naive cost now includes the additional round trips of sites whose
  demand is split across multiple runs. Previously the saving could
  become negative.

- [`cvrplib_download()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_download.md),
  [`clarke_wright_cvrplib()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright_cvrplib.md):
  Instances defined by an explicit distance matrix (including `E-n13-k4`
  and `E-n31-k7` in `cvrplib_E`) got corrupted distances, since
  coordinates were derived from them incorrectly. The matrix is now kept
  in the new field `distances` and used by the solver, and the
  coordinates in `pos` are `NA` for those instances.

- [`cvrplib_ls()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_ls.md),
  [`cvrplib_download()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_download.md):
  Upstream website has changed. Fix malfunctioning functions by
  adjusting webscraping accordingly.

## heumilkr 0.3.0

CRAN release: 2025-04-24

### Breaking changes

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md):
  The default argument of `restrictions` is changed from `NULL` to an
  empty data frame. Passing `restrictions = NULL` now fails.

### Minor improvements and fixes

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md):

  - Vehicle restrictions are now correctly applied if two tours are
    linked whose end points do not have vehicle restrictions maintained,
    but the tours include other edges with vehicle restrictions.

  - R session does not crash anymore due to integer overflow issues in
    certain circumstances.

- [`plot()`](https://rdrr.io/r/graphics/plot.default.html)/`autoplot()`:

  - Plotting of a milk run result with a single site now also works.

  - The functions do have reference pages now.

## heumilkr 0.2.0

CRAN release: 2024-04-01

### New features

- Add the `cvrp_*()` function family for handling
  [CVRPLIB](https://galgos.inf.puc-rio.br/cvrplib/) data.

- New vignette with performance benchmarks:
  [`vignette("clarke_wright_performance")`](https://lschneiderbauer.github.io/heumilkr/articles/clarke_wright_performance.md).

### Minor improvements and fixes

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  does not crash R anymore when passing `NA` values to the `distances`
  argument: it stops gracefully instead.

## heumilkr 0.1.0

CRAN release: 2024-01-10

- Initial CRAN submission.
