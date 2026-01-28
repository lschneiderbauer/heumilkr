# Package index

## Clarke-Wright algorithm

Functions for applying the Clarke-Wright algorithm either to custom
input or to CVRPLIB instance data

- [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  : Clarke-Wright algorithm, a Capacitated Vehicle Routing Problem
  solver

- [`clarke_wright_cvrplib()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright_cvrplib.md)
  :

  Apply
  [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  to CVRPLIB data

## Plotting

- [`autoplot(`*`<heumilkr_solution>`*`)`](https://lschneiderbauer.github.io/heumilkr/reference/autoplot.heumilkr_solution.md)
  : Create ggplot for a CVRP solution
- [`plot(`*`<heumilkr_solution>`*`)`](https://lschneiderbauer.github.io/heumilkr/reference/plot.heumilkr_solution.md)
  : Plot a CVRP solution

## Costs and Savings

Helper functions to retrieve costs as well as savings from CVRP
solutions

- [`milkr_cost()`](https://lschneiderbauer.github.io/heumilkr/reference/milkr_cost.md)
  : Vehicle runs cost / distance
- [`milkr_saving()`](https://lschneiderbauer.github.io/heumilkr/reference/milkr_saving.md)
  : Vehicle run saving

## Handling CVRPLIB data

Functions for listing and downloading CVRPLIB instance data

- [`clarke_wright_cvrplib()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright_cvrplib.md)
  :

  Apply
  [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
  to CVRPLIB data

- [`cvrplib_download()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_download.md)
  : CVRPLIB problem instance downloader

- [`cvrplib_ls()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_ls.md)
  : List available CVRPLIB online data

### Selected problem instances from CVRPLIB

- [`cvrplib_A`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_A.md)
  : CVRP instance data by Augerat, 1995
- [`cvrplib_B`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_B.md)
  : CVRP instance data by Augerat, 1995
- [`cvrplib_E`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_E.md)
  : CVRP instance data by Christofides and Eilon, 1969
- [`cvrplib_F`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_F.md)
  : CVRP instance data by Fisher, 1994
- [`cvrplib_Tai`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_Tai.md)
  : CVRP instance data by Rochat and Taillard, 1995
