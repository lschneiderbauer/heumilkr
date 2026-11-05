# Apply [`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md) to CVRPLIB data

Apply
[`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md)
to CVRPLIB data

## Usage

``` r
clarke_wright_cvrplib(instance)
```

## Arguments

- instance:

  A "`cvrplib_instance`" object. See
  [`cvrplib_download()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_download.md)
  or bundled CVRPLIB data like
  [cvrplib_A](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_A.md).

## Value

A "`heumilkr_solution`" object. See
[`clarke_wright()`](https://lschneiderbauer.github.io/heumilkr/reference/clarke_wright.md).

## See also

Other cvrplib:
[`cvrplib_download()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_download.md),
[`cvrplib_ls()`](https://lschneiderbauer.github.io/heumilkr/reference/cvrplib_ls.md)

## Examples

``` r
clarke_wright_cvrplib(cvrplib_A[[1]])
#> $runs
#>   run vehicle max_load  distance
#> 1   0       0       90 111.92186
#> 2   1       0       91 195.04820
#> 3   2       0       99 210.02747
#> 4   3       0       38  65.60087
#> 5   4       0       92 257.89977
#> 
#> $sites
#>    site demand
#> 1     0     19
#> 2     1     21
#> 3     2      6
#> 4     3     19
#> 5     4      7
#> 6     5     12
#> 7     6     16
#> 8     7      6
#> 9     8     16
#> 10    9      8
#> 11   10     14
#> 12   11     21
#> 13   12     16
#> 14   13      3
#> 15   14     22
#> 16   15     18
#> 17   16     19
#> 18   17      1
#> 19   18     24
#> 20   19      8
#> 21   20     12
#> 22   21      4
#> 23   22      8
#> 24   23     24
#> 25   24     24
#> 26   25      2
#> 27   26     20
#> 28   27     15
#> 29   28      2
#> 30   29     14
#> 31   30      9
#> 
#> $visits
#>    run site order load
#> 1    0    0     3   21
#> 2    2    1     4   14
#> 3    2    2     5    8
#> 4    4    3     4   43
#> 5    1    4     1   76
#> 6    4    5     1   78
#> 7    0    6     1   56
#> 8    4    7     6   23
#> 9    4    8     7    7
#> 10   1    9     3   44
#> 11   4   10     5   29
#> 12   0   11     4    0
#> 13   0   12     2   40
#> 14   4   13     9    0
#> 15   1   14     4   22
#> 16   0   15     0   72
#> 17   2   16     3   35
#> 18   4   17     2   77
#> 19   2   18     2   54
#> 20   1   19     0   83
#> 21   2   20     0   87
#> 22   4   21     8    3
#> 23   2   22     6    0
#> 24   3   23     0   14
#> 25   1   24     2   52
#> 26   4   25     0   90
#> 27   1   26     6    0
#> 28   4   27     3   62
#> 29   1   28     5   20
#> 30   3   29     1    0
#> 31   2   30     1   78
#> 
```
