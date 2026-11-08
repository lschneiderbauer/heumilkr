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
        RuntimeError: If there are not enough vehicles available to start
            with one run per site.
    """
    demand_array = _as_float_array(demand)
    if demand_array.ndim != 1 or demand_array.size == 0:
        raise ValueError("demand must be a non-empty one-dimensional sequence")

    n_sites = demand_array.size
    condensed = _condensed_distances(distances, n_sites)

    vehicles = tuple(vehicles)
    counts = [_vehicle_count(vehicle) for vehicle in vehicles]
    capacities = [float(vehicle.capacity) for vehicle in vehicles]

    restrictions = tuple(restrictions)
    restr_sites = [_as_c_int(restriction.site) for restriction in restrictions]
    restr_vehicles = [_as_c_int(restriction.vehicle) for restriction in restrictions]

    # the core checks the values, see _input_error() for the messages
    try:
        _, run_table, visit_table = _core.clarke_wright(
            demand_array, condensed, counts, capacities, restr_sites, restr_vehicles
        )
    except _core.InputError as error:
        raise _input_error(
            *error.args, demand=demand_array, vehicles=vehicles, restrictions=restrictions
        ) from None

    return Solution(
        runs=_build_runs(run_table, visit_table),
        demand=demand_array,
        # the first n_sites entries of the condensed form belong to the depot
        depot_distances=condensed[:n_sites],
    )


def _as_float_array(values: Any) -> np.ndarray:
    array = np.array(values, dtype=np.float64, order="C")
    array.flags.writeable = False
    return array


def _condensed_distances(distances: Any, n_sites: int) -> np.ndarray:
    array = _as_float_array(distances)
    n_locations = n_sites + 1

    # the core checks the length of the condensed form
    if array.ndim == 1:
        return array

    if array.shape != (n_locations, n_locations):
        raise ValueError(
            f"distances must have shape ({n_locations}, {n_locations}) for "
            f"{n_sites} sites, got {array.shape}"
        )
    # NaN is reported by the core, together with its location
    if not np.allclose(array, array.T, equal_nan=True):
        raise ValueError("distance matrix must be symmetric")

    condensed = array[np.triu_indices(n_locations, k=1)]
    condensed.flags.writeable = False
    return condensed


def _vehicle_count(vehicle: Vehicle) -> int:
    if vehicle.count is None:
        return _UNLIMITED
    return _as_c_int(vehicle.count)


def _as_c_int(value: Any) -> int:
    # Out of range values are clamped, so that they reach the core and are
    # reported there. Messages refer to the original value.
    return min(max(operator.index(value), -_UNLIMITED - 1), _UNLIMITED)


def _input_error(
    code: str,
    index: int,
    other_index: int,
    value: float,
    *,
    demand: np.ndarray,
    vehicles: Sequence[Vehicle],
    restrictions: Sequence[Restriction],
) -> Exception:
    """Turns an error of the C++ core into an exception with a message.

    See src/input_error.h for the meaning of the arguments per code.
    """
    if code == "non_finite_demand":
        return ValueError(
            "demand must not contain NaN or infinite values, "
            f"got {value} at index {index}"
        )
    if code == "wrong_distance_count":
        return ValueError(
            f"condensed distances must have {other_index} entries for "
            f"{demand.size} sites, got {index}"
        )
    if code == "non_finite_distance":
        return ValueError(
            "distances must not contain NaN or infinite values, "
            f"got {value} between locations {index} and {other_index}"
        )
    if code == "no_vehicle_types":
        return ValueError("vehicles must contain at least one vehicle type")
    if code == "negative_vehicle_count":
        return ValueError(
            "vehicle count must not be negative, "
            f"got {vehicles[index].count} for vehicles[{index}]"
        )
    if code == "invalid_vehicle_capacity":
        return ValueError(
            "vehicle capacity must be positive and finite, "
            f"got {vehicles[index].capacity} for vehicles[{index}]"
        )
    if code == "unknown_restriction_site":
        return ValueError(
            f"restrictions[{index}] refers to unknown site "
            f"{restrictions[index].site}, {_numbering(demand.size, 'site')}"
        )
    if code == "unknown_restriction_vehicle":
        return ValueError(
            f"restrictions[{index}] refers to unknown vehicle "
            f"{restrictions[index].vehicle}, "
            f"{_numbering(len(vehicles), 'vehicle type')}"
        )
    if code == "insufficient_vehicles":
        return RuntimeError(
            "Not enough vehicles available to start with one run per site, "
            f"ran out at site {index}. The solver merges runs only afterwards, "
            "so it can need more vehicles at the start than in the final "
            "solution. A vehicle type with count=None and without restrictions "
            "avoids this."
        )

    # vehicle_length_mismatch, restriction_length_mismatch, and codes that
    # this function does not know (yet)
    return RuntimeError(
        f"internal error of heumilkr, please report it: {code} "
        f"({index}, {other_index}, {value})"
    )


def _numbering(n: int, what: str) -> str:
    """For example "sites are numbered from 0 to 4"."""
    if n == 1:
        return f"the only {what} is 0"
    return f"{what}s are numbered from 0 to {n - 1}"


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
