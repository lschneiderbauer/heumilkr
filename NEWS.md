# heumilkr 0.2.0

## Enhancements

-   add the `cvrp_*()` function family for handling [CVRPLIB](http://vrp.atd-lab.inf.puc-rio.br/) data.

-   new vignette with performance benchmarks: `vignette("clarke_wright_performance")`.

## Bugfixes

-   Fix `clarke_wright()` crashing R when passing `NA` values to the `distances` argument: it stops gracefully instead.

# heumilkr 0.1.0

-   Initial CRAN submission.
