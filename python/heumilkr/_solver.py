from __future__ import annotations

import math
import operator
from collections.abc import Iterable, Sequence
from dataclasses import dataclass, field
from typing import TYPE_CHECKING, Any

import numpy as np

from . import _core

if TYPE_CHECKING:
    import pandas as pd

# The solver treats this number of vehicles as "unlimited".
_UNLIMITED = 2**31 - 1


@dataclass(frozen=True, slots=True)
class Vehicle:
    """A vehicle type.

    Attributes:
        capacity: Capacity of the vehicle, in the same units as the demand.
        count: Number of available vehicles, or ``None`` if unlimited. Keeping
            at least one type unlimited is recommended: the solver starts with
            one run per site and may otherwise run out of vehicles, even if the
            final solution would fit.
    """

    capacity: float
    count: int | None = None


@dataclass(frozen=True, slots=True)
class Restriction:
    """Forbids a vehicle type from approaching a site.

    Attributes:
        vehicle: Index of the vehicle type in ``vehicles``, starting at 0.
        site: Index of the site in ``demand``, starting at 0.
    """

    vehicle: int
    site: int


@dataclass(frozen=True, slots=True)
class Run:
    """A single tour, which starts and ends at the depot.

    Attributes:
        id: Run identifier, equal to the position in ``Solution.runs``.
        vehicle: Index of the vehicle type in ``vehicles``, starting at 0.
        sites: Indices of the visited sites, in visiting order.
        loads: Load of the vehicle after each visit, matching ``sites``.
        distance: Travel distance of the run.
        max_load: Maximal load on the run, at most the vehicle capacity.
    """

    id: int
    vehicle: int
    sites: tuple[int, ...]
    loads: tuple[float, ...]
    distance: float
    max_load: float


@dataclass(frozen=True, eq=False)
class Solution:
    """The result of :func:`clarke_wright`.

    A site whose demand exceeds the vehicle capacities is split across multiple
    runs. Every other site is part of exactly one run.

    Attributes:
        runs: The runs of the solution.
        demand: The demand of each site, as supplied to the solver.
        depot_distances: The distance between the depot and each site.
    """

    runs: tuple[Run, ...]
    demand: np.ndarray
    depot_distances: np.ndarray = field(repr=False)

    def cost(self) -> float:
        """Returns the total travel distance, which the solver minimizes."""
        return math.fsum(run.distance for run in self.runs)

    def saving(self, relative: bool = False) -> float:
        """Returns the saving compared to the naive solution.

        The naive solution uses one round trip per visit, i.e. one per site,
        or as many as the solution uses for a site with split demand.

        Args:
            relative: Return the saving as a fraction of the naive cost, instead
                of in units of distance. It is NaN if the naive cost is zero.
        """
        naive_cost = math.fsum(
            2 * float(self.depot_distances[site])
            for run in self.runs
            for site in run.sites
        )
        saving = naive_cost - self.cost()

        if not relative:
            return saving
        return saving / naive_cost if naive_cost else math.nan

    def to_pandas(self) -> dict[str, pd.DataFrame]:
        """Returns the solution as data frames. Requires pandas.

        Returns:
            A dictionary with the data frames ``"runs"`` (columns ``run``,
            ``vehicle``, ``max_load``, ``distance``), ``"sites"`` (``site``,
            ``demand``) and ``"visits"`` (``run``, ``site``, ``order``,
            ``load``).
        """
        try:
            import pandas as pd
        except ImportError as error:
            raise ImportError(
                "to_pandas() requires pandas: pip install heumilkr[pandas]"
            ) from error

        visits = [
            (run.id, site, order, load)
            for run in self.runs
            for order, (site, load) in enumerate(zip(run.sites, run.loads))
        ]

        return {
            "runs": pd.DataFrame(
                [(r.id, r.vehicle, r.max_load, r.distance) for r in self.runs],
                columns=["run", "vehicle", "max_load", "distance"],
            ),
            "sites": pd.DataFrame(
                {"site": np.arange(len(self.demand)), "demand": self.demand}
            ),
            "visits": pd.DataFrame(
                visits, columns=["run", "site", "order", "load"]
            ),
        }


def clarke_wright(
    demand: Any,
    distances: Any,
    vehicles: Sequence[Vehicle],
    restrictions: Iterable[Restriction] = (),
) -> Solution:
    """Finds a quasi-optimal solution to the Capacitated Vehicle Routing Problem.

    All runs start and end at a single depot. See Clarke, G. and Wright, J.R.
    (1964), https://doi.org/10.1287/opre.12.4.568.

    Args:
        demand: The demand of each of the ``N`` sites. Positive values are goods
            delivered from the depot to the site, negative values are goods
            picked up at the site and brought back to the depot.
        distances: Distances between the ``N + 1`` locations, where the first
            location is the depot. Either a symmetric ``(N + 1, N + 1)`` matrix,
            or its condensed form as returned by ``scipy.spatial.distance.pdist``.
        vehicles: The available vehicle types. If several types are eligible for
            a run, the first one is chosen, so cheaper ones should come first.
        restrictions: Vehicle types that can not approach particular sites.

    Returns:
        The solution. Identical input always yields an identical solution.

    Raises:
        ValueError: If the input is invalid.
        RuntimeError: If there are not enough vehicles available.
    """
    demand_array = _as_float_array(demand, "demand")
    if demand_array.ndim != 1 or demand_array.size == 0:
        raise ValueError("demand must be a non-empty one-dimensional sequence")

    n_sites = demand_array.size
    condensed = _condensed_distances(distances, n_sites)

    vehicles = list(vehicles)
    if not vehicles:
        raise ValueError("vehicles must contain at least one vehicle type")

    counts = [_vehicle_count(vehicle) for vehicle in vehicles]
    capacities = [_vehicle_capacity(vehicle) for vehicle in vehicles]

    restr_sites = []
    restr_vehicles = []
    for restriction in restrictions:
        site = operator.index(restriction.site)
        vehicle = operator.index(restriction.vehicle)
        if not 0 <= site < n_sites:
            raise ValueError(f"restriction refers to unknown site {site}")
        if not 0 <= vehicle < len(vehicles):
            raise ValueError(f"restriction refers to unknown vehicle {vehicle}")
        restr_sites.append(site)
        restr_vehicles.append(vehicle)

    _, run_table, visit_table = _core.clarke_wright(
        demand_array, condensed, counts, capacities, restr_sites, restr_vehicles
    )

    return Solution(
        runs=_build_runs(run_table, visit_table),
        demand=demand_array,
        # the first n_sites entries of the condensed form belong to the depot
        depot_distances=condensed[:n_sites],
    )


def _as_float_array(values: Any, name: str) -> np.ndarray:
    array = np.array(values, dtype=np.float64, order="C")
    if not np.all(np.isfinite(array)):
        raise ValueError(f"{name} must not contain NaN or infinite values")
    array.flags.writeable = False
    return array


def _condensed_distances(distances: Any, n_sites: int) -> np.ndarray:
    array = _as_float_array(distances, "distances")
    n_locations = n_sites + 1

    if array.ndim == 1:
        expected = n_locations * n_sites // 2
        if array.size != expected:
            raise ValueError(
                f"condensed distances must have {expected} entries for "
                f"{n_sites} sites, got {array.size}"
            )
        return array

    if array.shape != (n_locations, n_locations):
        raise ValueError(
            f"distances must have shape ({n_locations}, {n_locations}) for "
            f"{n_sites} sites, got {array.shape}"
        )
    if not np.allclose(array, array.T):
        raise ValueError("distance matrix must be symmetric")

    condensed = array[np.triu_indices(n_locations, k=1)]
    condensed.flags.writeable = False
    return condensed


def _vehicle_count(vehicle: Vehicle) -> int:
    if vehicle.count is None:
        return _UNLIMITED

    count = operator.index(vehicle.count)
    if count < 0:
        raise ValueError(f"vehicle count must not be negative, got {count}")
    return min(count, _UNLIMITED)


def _vehicle_capacity(vehicle: Vehicle) -> float:
    capacity = float(vehicle.capacity)
    if not (capacity > 0 and math.isfinite(capacity)):
        raise ValueError(
            f"vehicle capacity must be positive and finite, got {capacity}"
        )
    return capacity


def _build_runs(run_table: tuple, visit_table: tuple) -> tuple[Run, ...]:
    visits_by_run: dict[int, list[tuple[int, int, float]]] = {}
    for run, site, order, load in zip(*visit_table):
        visits_by_run.setdefault(run, []).append((order, site, load))

    runs = []
    for run, vehicle, max_load, distance in zip(*run_table):
        visits = sorted(visits_by_run[run])
        runs.append(
            Run(
                id=run,
                vehicle=vehicle,
                sites=tuple(visit[1] for visit in visits),
                loads=tuple(visit[2] for visit in visits),
                distance=distance,
                max_load=max_load,
            )
        )
    return tuple(runs)
