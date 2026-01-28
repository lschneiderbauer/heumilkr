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
#> 1   0       0       90 122.04521
#> 2   1       0       99 210.02747
#> 3   2       0       92 286.63383
#> 4   3       0       91 222.24300
#> 5   4       0       38  65.60087
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
#> 1    0    0     2   32
#> 2    1    1     4   14
#> 3    1    2     5    8
#> 4    2    3     9    0
#> 5    3    4     2   56
#> 6    2    5     2   75
#> 7    0    6     3   16
#> 8    2    7     4   54
#> 9    2    8     7   33
#> 10   3    9     5   24
#> 11   2   10     8   19
#> 12   0   11     1   51
#> 13   0   12     4    0
#> 14   2   13     1   87
#> 15   3   14     4   32
#> 16   0   15     0   72
#> 17   1   16     3   35
#> 18   2   17     5   53
#> 19   1   18     2   54
#> 20   3   19     1   63
#> 21   1   20     0   87
#> 22   2   21     6   49
#> 23   1   22     6    0
#> 24   4   23     1    0
#> 25   3   24     6    0
#> 26   2   25     0   90
#> 27   3   26     0   71
#> 28   2   27     3   60
#> 29   3   28     3   54
#> 30   4   29     0   24
#> 31   1   30     1   78
#> 
```
