# Plot a CVRP solution

Represents the sites and runs on a 2D plane so that the distances
between sites on the drawn 2D plane correspond to `distances` provided
to the solver
[`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md).

The individual runs are distinguished by color. The demanding site
locations are marked with round circles while the (single) supplying
site is depicted as a square. The line types (solid/dashed/...) are
associated to different vehicle types.

## Usage

``` r
# S3 method for class 'heumilkr_solution'
plot(x, ...)
```

## Arguments

- x:

  A "`heumilkr_solution`" object, typically obtained by
  [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md).

- ...:

  Not used.

## Details

Distance information between sites only determine site positions on a 2D
plane up to rotations and translations: those are fixed arbitrarily.
