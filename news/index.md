# Changelog

## heumilkr (development version)

### Breaking changes

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md):
  Return value changed: instead of returning a single
  [`data.frame()`](https://rdrr.io/r/base/data.frame.html) a list of
  three [`data.frame()`](https://rdrr.io/r/base/data.frame.html)s is
  returned now that makes it easier to interpret the result.

- [`cvrplib_ls()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_ls.md)
  now returns a [`data.frame()`](https://rdrr.io/r/base/data.frame.html)
  instead of a simple vector.

### New features / improvements

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  now supports negative demands: sites with negative demand are treated
  as “backhaul” points, i.e. they deliver goods back to the depot. By
  design they can only be visited after all positive-demand sites have
  been served.

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  has gained runtime speedup compared to the previous version by roughly
  a factor of 3 (the actual speedup depends on input characteristics).

### Bugfixes

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
  [CVRPLIB](http://vrp.atd-lab.inf.puc-rio.br/) data.

- New vignette with performance benchmarks:
  [`vignette("clarke_wright_performance")`](https://lschneiderbauer.github.io/heumilkr/articles/clarke_wright_performance.md).

### Minor improvements and fixes

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  does not crash R anymore when passing `NA` values to the `distances`
  argument: it stops gracefully instead.

## heumilkr 0.1.0

CRAN release: 2024-01-10

- Initial CRAN submission.
