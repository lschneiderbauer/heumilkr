# List available CVRPLIB online data

Scrapes the CVRPLIB website to look for available data sets. This
function call can take some time.

## Usage

``` r
cvrplib_ls(cvrplib_url = "https://galgos.inf.puc-rio.br")
```

## Arguments

- cvrplib_url:

  The base URL of the CVRPLIB website. Defaults to
  https://galgos.inf.puc-rio.br.

## Value

A [`data.frame()`](https://rdrr.io/r/base/data.frame.html) where each
row represents a CVRPLIB instance with the following columns:

- `name` - Instance qualifiers which can be used with
  [`cvrplib_download()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_download.md).

- `instance_url` - The URL to the instance file.

- `solution_url` - The URL to the solution file.

## See also

Other cvrplib:
[`clarke_wright_cvrplib()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright_cvrplib.md),
[`cvrplib_download()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_download.md)
