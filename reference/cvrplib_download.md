# CVRPLIB problem instance downloader

[CVRLIB](https://galgos.inf.puc-rio.br/) offers a selection of CVRP
problem instances. This function downloads the instance data and
conveniently makes it available to be fed into solver functions, e.g.
with
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
instance data.

## See also

Other cvrplib:
[`clarke_wright_cvrplib()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright_cvrplib.md),
[`cvrplib_ls()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_ls.md)
