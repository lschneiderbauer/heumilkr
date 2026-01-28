# Vehicle run saving

Measures the saving that was achieved by the heuristic optimization
algorithm
[`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
compared to the naive vehicle run assignment, i.e. one run per site.

## Usage

``` r
milkr_saving(solution, relative = FALSE)
```

## Arguments

- solution:

  A "`heumilkr_solution`" object, typically obtained by
  [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md).

- relative:

  Should the saving be given as dimensionful value (in units of distance
  as provided to
  [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)),
  or as percentage relative to the naive costs. Defaults to `FALSE`,
  i.e. a dimensionful value.

## Value

The savings either as dimensionful value or as percentage relative to
the naive costs, depending on `relative`.

## Examples

``` r
demand <- c(3, 2, 4, 2)

positions <-
  data.frame(
    pos_x = c(0, 1, -1, 2, 3),
    pos_y = c(0, 1, 1, 2, 3)
  )

solution <- clarke_wright(
  demand,
  dist(positions),
  data.frame(n = NA_integer_, caps = 6)
)

print(milkr_saving(solution))
#> [1] 6.485281
print(milkr_saving(solution, relative = TRUE))
#> [1] 0.3275562
```
