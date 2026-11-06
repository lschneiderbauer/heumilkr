# heumilkr

This Python package provides an implementation of the Clarke-Wright algorithm to
find a quasi-optimal solution to the
[Capacitated Vehicle Routing Problem](https://en.wikipedia.org/wiki/Vehicle_routing_problem).
It shares its C++ core with the
[R package of the same name](https://github.com/lschneiderbauer/heumilkr).

Besides deliveries, it also supports "backhaul": sites with negative demand have
goods picked up and brought back to the depot, in any order within a run as long
as the load stays within the vehicle capacity.

## Installation

```sh
pip install heumilkr
```

The only dependency is NumPy. Install `heumilkr[pandas]` to also get pandas for
`Solution.to_pandas()`.

## Example

```python
import heumilkr as hm

solution = hm.clarke_wright(
    # sites with positive demand get deliveries, others get a pick-up
    demand=[3, 2, 4, -2],
    # distances between all locations, the first one is the depot
    distances=[
        [0, 1, 1, 2, 3],
        [1, 0, 2, 1, 2],
        [1, 2, 0, 3, 4],
        [2, 1, 3, 0, 1],
        [3, 2, 4, 1, 0],
    ],
    vehicles=[
        hm.Vehicle(capacity=6, count=2),  # two vehicles with capacity 6
        hm.Vehicle(capacity=9),  # unlimited vehicles with capacity 9
    ],
    # the first vehicle type can not approach the third site
    restrictions=[hm.Restriction(vehicle=0, site=2)],
)

for run in solution.runs:
    print(run.vehicle, run.sites, run.distance, run.max_load)

solution.cost()  # the total distance, which the solver minimizes
solution.saving()  # compared to one round trip per site
solution.saving(relative=True)  # the same, as a fraction

solution.to_pandas()["visits"]  # data frames, if pandas is installed
```

`distances` can be a square matrix, or the condensed form that
`scipy.spatial.distance.pdist` returns. All indices start at 0.

## License

GPL (>= 3)
