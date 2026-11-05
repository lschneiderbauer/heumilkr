# Clarke-Wright algorithm, a Capacitated Vehicle Routing Problem solver

Finds a quasi-optimal solution to the Capacitated Vehicle Routing
Problem (CVRP). All runs start and end at a single depot (origin). Goods
are delivered from the depot to sites with positive demand, and picked
up from sites with negative demand and brought back to the depot
("backhaul").

## Usage

``` r
clarke_wright(
  demand,
  distances,
  vehicles,
  restrictions = data.frame(vehicle = integer(), site = integer())
)
```

## Arguments

- demand:

  A [`numeric()`](https://rdrr.io/r/base/numeric.html) vector consisting
  of "demands" indexed by sites. The `i`th entry refers to the demand of
  the `i`th site. The length of the vector equals the number of sites
  `N`. The units of demand values need to match the units of vehicle
  capacity values. `NA` values are not allowed. Positive values are
  goods to be delivered from the depot to a site. Negative values are
  goods to be picked up at a site and brought to the depot, i.e.
  "backhaul". Sites with positive and negative demands can be visited in
  any order within a run, as long as the load stays within the vehicle
  capacity at every point of the run. Goods picked up at one site can
  not be delivered to another site. Sites with a demand of zero are
  still visited.

- distances:

  An object of class `dist`, created by
  [`stats::dist()`](https://rdrr.io/r/stats/dist.html), with `(N + 1)`
  locations describing the distances between individual sites. The first
  index refers to the depot. The `(i+1)`th index refers to the `i`th
  site (as defined by `demand`).

- vehicles:

  A [`data.frame()`](https://rdrr.io/r/base/data.frame.html) describing
  available vehicle types and their respective capacities. One row per
  vehicle type. The data frame is expected to have two columns:

  - `n` - Number of available vehicles as an
    [`integer()`](https://rdrr.io/r/base/integer.html). This can be set
    to `NA_integer_` if the number is "infinite" (i.e. effectively the
    maximal integer value on your machine.). It is recommended to keep
    at least one vehicle type as "infinite", otherwise the solver might
    raise a run time error due to initially not having enough vehicles
    available (even though the final solution might satisfy the
    availability restrictions).

  - `caps` - The vehicle capacity in same units as `demand`.

  The order of the
  [`data.frame()`](https://rdrr.io/r/base/data.frame.html) is relevant
  and determines the prioritization of vehicle assignments to runs (in
  case two or more vehicle types are eligible for assignment the "first"
  vehicle is chosen). In a typical scenario "more expensive" vehicles
  should be further down in the list (so the cheaper one is chosen in
  case there is doubt). Since higher capacity vehicles usually involve
  higher costs sorting the data frame by capacity is usually a good rule
  of thumb.

- restrictions:

  An optional [`data.frame()`](https://rdrr.io/r/base/data.frame.html)
  that allows to define vehicle type restrictions for particular sites
  in the form of a blacklist. The data frame is expected to have two
  columns:

  - `vehicle` - The vehicle type index as an
    [`integer()`](https://rdrr.io/r/base/integer.html), starting at 0
    (i.e. 0 refers to the first row of `vehicles`).

  - `site` - The site index as an
    [`integer()`](https://rdrr.io/r/base/integer.html), starting at 0
    (i.e. 0 refers to the first entry of `demand`).

  Each row defines a restriction: vehicle type `vehicle` can not
  approach site `site`. Defaults to an empty
  [`data.frame()`](https://rdrr.io/r/base/data.frame.html), i.e. no
  restrictions are enforced.

## Value

Returns a "`heumilkr_solution`" object, a named
[`list()`](https://rdrr.io/r/base/list.html) of three
[`data.frame()`](https://rdrr.io/r/base/data.frame.html)s with names
"runs", "sites" and "visits", bestowed with additional attributes.

- `runs`: Each record reflects a single run/tour.

  - `run` - A run identifier.

  - `vehicle` - The vehicle type index associated to the run, starting
    at 0 (i.e. 0 refers to the first row of `vehicles`).

  - `max_load` - The maximal load of the vehicle on this run. It is
    always smaller or equal the vehicle capacity.

  - `distance` - The travel distance of the particular run.

- `sites`: Each record reflects a site. Its columns consist of:

  - `site` - The site index, starting at 0 (i.e. 0 refers to the first
    entry of `demand`).

  - `demand` - The site demand. This is supplied as an input.

- `visits`: Each record reflects a run visit of a particular site. It is
  implied that each run starts and ends at the origin. Its columns
  consist of:

  - `run` - Identifies the run the site is assigned to.

  - `site` - The site index associated to the run, starting at 0 (i.e. 0
    refers to the first entry of `demand`).

  - `order` - Integer values providing the visiting order within each
    run.

  - `load` - The load after visiting site `site` in units of `demand`
    per particular run.

Unless a site demand exceeds the vehicle capacities it is always
assigned to only a single run. Otherwise it is split across multiple
runs, and the site appears once per run in `visits`. `load` and
`max_load` then refer to the share of the demand served by the
particular run, while `demand` in `sites` still holds the full demand.

## Details

See the original paper, Clarke, G. and Wright, J.R. (1964)
[doi:10.1287/opre.12.4.568](https://doi.org/10.1287/opre.12.4.568) , for
a detailed explanation of the Clarke-Wright algorithm.

To support deliveries and pickups in any order within a run, two runs
can only be merged by connecting the end of one run to the start of the
other. The original algorithm can connect either end of a run, but
reversing a run with both deliveries and pickups would change its load
along the way.

As an additional step after merging, the visiting order is re-optimized
with a greedy heuristic for runs that either only deliver or only pick
up. Runs with both keep their order, so that the load stays within the
vehicle capacity.

## Examples

``` r
demand <- c(3, 2, 4, 2)

positions <-
  data.frame(
    pos_x = c(0, 1, -1, 2, 3),
    pos_y = c(0, 1, 1, 2, 3)
  )

clarke_wright(
  demand,
  dist(positions),
  data.frame(n = NA_integer_, caps = 6)
)
#> $runs
#>   run vehicle max_load distance
#> 1   0       0        5 4.828427
#> 2   1       0        6 8.485281
#> 
#> $sites
#>   site demand
#> 1    0      3
#> 2    1      2
#> 3    2      4
#> 4    3      2
#> 
#> $visits
#>   run site order load
#> 1   0    0     1    0
#> 2   0    1     0    3
#> 3   1    2     0    2
#> 4   1    3     1    0
#> 
```
