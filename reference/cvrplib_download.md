# CVRPLIB problem instance downloader

[CVRPLIB](https://galgos.inf.puc-rio.br/cvrplib/en/instances) offers a
selection of CVRP problem instances. This function downloads the
instance data and conveniently makes it available to be fed into solver
functions, e.g. with
[`clarke_wright_cvrplib()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright_cvrplib.md).
The primary purpose for those instances is benchmarking / comparing
speed as well as performance of solvers.

## Usage

``` r
cvrplib_download(qualifier, cvrplib_url = "https://galgos.inf.puc-rio.br")
```

## Arguments

- qualifier:

  The qualifier of the problem instance. E.g. "tai150d". This can either
  be inferred directly from the website or by the output of
  [`cvrplib_ls()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_ls.md).

- cvrplib_url:

  The base URL of the CVRPLIB website. Defaults to
  https://galgos.inf.puc-rio.br.

## Value

Returns a "`cvrplib_instance`" object which contains CVRPLIB problem
instance data. Its `best_known_cost` is the cost of the best known
solution provided by CVRPLIB (not necessarily a proven optimum), or `NA`
if CVRPLIB provides no solution file for the instance. A solution file
that exists but can not be downloaded raises an error.

Instances defined by an explicit distance matrix store it as
`distances`, a `dist` object, and have `NA` coordinates in `pos`.
Instances defined by coordinates have `distances` set to `NULL`.

## See also

Other cvrplib:
[`clarke_wright_cvrplib()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright_cvrplib.md),
[`cvrplib_ls()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_ls.md)
