"""Reference of the **Python** package heumilkr, a heuristic Capacitated Vehicle
Routing Problem solver.

This site only documents the Python package. The R package of the same name
shares its C++ core and has [its own site](https://lschneiderbauer.github.io/heumilkr/).

The [performance benchmarks](https://lschneiderbauer.github.io/heumilkr/articles/clarke_wright_performance.html) are part of the R package site, as they were
measured through it. They apply to the Python package as well, since they
measure the shared solver.

## Example

```python
import heumilkr as hm

solution = hm.clarke_wright(
    demand=[3, 2, 4, 2],
    # distances between all locations, the first one is the depot
    distances=[
        [0, 1, 1, 2, 3],
        [1, 0, 2, 1, 2],
        [1, 2, 0, 3, 4],
        [2, 1, 3, 0, 1],
        [3, 2, 4, 1, 0],
    ],
    # one vehicle with capacity 6, and an unlimited number with capacity 9
    vehicles=[hm.Vehicle(capacity=6, count=1), hm.Vehicle(capacity=9)],
)

for run in solution.runs:
    print(run.vehicle, run.sites, run.distance, run.max_load)

solution.cost()
solution.saving()
```
"""

from importlib.metadata import version

from ._solver import Restriction, Run, Solution, Vehicle, clarke_wright

__all__ = ["Restriction", "Run", "Solution", "Vehicle", "clarke_wright"]
__version__ = version("heumilkr")
