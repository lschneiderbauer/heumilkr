# Vehicle runs cost / distance

Calculates the total distance associated to a
[`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
result. This is the measure that the corresponding Capacitated Vehicle
Routing Problem minimizes.

## Usage

``` r
milkr_cost(solution)
```

## Arguments

- solution:

  A "`heumilkr_solution`" object, typically obtained by
  [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md).

## Value

The total traveled distance.

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

milkr_cost(solution)
#> [1] 13.31371
```
