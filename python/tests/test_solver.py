"""Tests of what the Python layer adds: input handling and the result objects.

The algorithm itself is tested in the R package (tests/testthat).
"""

import dataclasses
import math
import sys

import numpy as np
import pytest

import heumilkr as hm

UNLIMITED = [hm.Vehicle(capacity=30)]


def line_distances(n_sites):
    """Distance matrix of the depot and sites at positions 0, 1, ..., n_sites."""
    positions = np.arange(n_sites + 1)
    return np.abs(positions[:, None] - positions[None, :]).astype(float)


def random_problem(seed, n_sites=30):
    rng = np.random.default_rng(seed)
    positions = rng.uniform(-10, 10, size=(n_sites + 1, 2))
    distances = np.linalg.norm(positions[:, None] - positions[None, :], axis=-1)
    demand = rng.uniform(1, 15, size=n_sites) * rng.choice([-1, 1], size=n_sites)
    return demand, distances


def condensed(matrix):
    return matrix[np.triu_indices(len(matrix), k=1)]


@pytest.mark.parametrize("seed", range(5))
def test_solution_is_consistent_with_its_input(seed):
    demand, distances = random_problem(seed)
    vehicles = [hm.Vehicle(capacity=35, count=2), hm.Vehicle(capacity=50)]
    restriction = hm.Restriction(vehicle=0, site=0)

    solution = hm.clarke_wright(demand, distances, vehicles, [restriction])

    visited = sorted(site for run in solution.runs for site in run.sites)
    assert visited == list(range(len(demand)))

    for position, run in enumerate(solution.runs):
        assert run.id == position
        assert len(run.loads) == len(run.sites)

        stops = [0, *(site + 1 for site in run.sites), 0]
        assert run.distance == pytest.approx(
            sum(distances[a, b] for a, b in zip(stops, stops[1:]))
        )

        capacity = vehicles[run.vehicle].capacity
        assert run.max_load <= capacity
        assert all(0 <= load <= run.max_load for load in run.loads)

        if 0 in run.sites:
            assert run.vehicle != 0


def test_split_demand_yields_one_run_per_vehicle():
    solution = hm.clarke_wright(
        [15], [[0, 1], [1, 0]], [hm.Vehicle(capacity=6)]
    )

    assert sorted(run.max_load for run in solution.runs) == [3, 6, 6]
    assert all(run.sites == (0,) for run in solution.runs)
    assert solution.cost() == 6
    assert solution.saving() == 0


def test_cost_and_saving():
    # two sites on a line: one run costs 4, separate round trips would cost 6
    solution = hm.clarke_wright([1, 1], line_distances(2), [hm.Vehicle(capacity=2)])

    assert len(solution.runs) == 1
    assert solution.cost() == 4
    assert solution.saving() == 2
    assert solution.saving(relative=True) == pytest.approx(1 / 3)


def test_relative_saving_without_any_distance_is_nan():
    solution = hm.clarke_wright([1], [[0, 0], [0, 0]], UNLIMITED)

    assert math.isnan(solution.saving(relative=True))


def test_square_and_condensed_distances_are_equivalent():
    demand, distances = random_problem(0)

    from_square = hm.clarke_wright(demand, distances, UNLIMITED)
    from_condensed = hm.clarke_wright(demand, condensed(distances), UNLIMITED)

    assert from_square.runs == from_condensed.runs
    assert from_square.saving() == from_condensed.saving()


def test_condensed_distances_match_scipy_pdist():
    pdist = pytest.importorskip("scipy.spatial.distance").pdist
    positions = np.random.default_rng(0).uniform(size=(6, 2))
    matrix = np.linalg.norm(positions[:, None] - positions[None, :], axis=-1)

    assert pdist(positions) == pytest.approx(condensed(matrix))


def test_sequences_and_non_contiguous_arrays_are_accepted():
    demand, distances = random_problem(0, n_sites=5)
    expected = hm.clarke_wright(demand, distances, UNLIMITED).runs

    assert hm.clarke_wright(demand.tolist(), distances.tolist(), UNLIMITED).runs == expected
    assert (
        hm.clarke_wright(
            np.repeat(demand, 2)[::2],
            np.repeat(distances, 2, axis=0)[::2],
            UNLIMITED,
        ).runs
        == expected
    )


def test_input_arrays_are_not_shared_with_the_solution():
    demand = np.array([1.0, 2.0])

    solution = hm.clarke_wright(demand, line_distances(2), UNLIMITED)
    demand[0] = 99

    assert solution.demand.tolist() == [1, 2]
    with pytest.raises(ValueError):
        solution.demand[0] = 99


def test_vehicle_order_and_count_are_respected():
    solution = hm.clarke_wright(
        [1, 1],
        [[0, 1, 100], [1, 0, 100], [100, 100, 0]],
        [hm.Vehicle(capacity=1, count=1), hm.Vehicle(capacity=1)],
    )

    assert sorted(run.vehicle for run in solution.runs) == [0, 1]


def test_not_enough_vehicles_raises():
    with pytest.raises(RuntimeError, match="Not enough vehicles"):
        hm.clarke_wright([15, 3], line_distances(2), [hm.Vehicle(capacity=6, count=2)])


@pytest.mark.parametrize(
    ("demand", "distances", "vehicles", "restrictions", "message"),
    [
        ([], [[0.0]], UNLIMITED, (), "demand must be"),
        ([[1, 2]], line_distances(2), UNLIMITED, (), "demand must be"),
        ([1, math.nan], line_distances(2), UNLIMITED, (), "demand must not"),
        ([1, math.inf], line_distances(2), UNLIMITED, (), "demand must not"),
        ([1, 2], line_distances(3), UNLIMITED, (), "shape"),
        ([1, 2], [1, 2], UNLIMITED, (), "entries"),
        ([1], [[0, math.nan], [math.nan, 0]], UNLIMITED, (), "distances must not"),
        ([1], [[0, 1], [2, 0]], UNLIMITED, (), "symmetric"),
        ([1], [1], [], (), "at least one"),
        ([1], [1], [hm.Vehicle(capacity=0)], (), "capacity"),
        ([1], [1], [hm.Vehicle(capacity=math.nan)], (), "capacity"),
        ([1], [1], [hm.Vehicle(capacity=1, count=-1)], (), "count"),
        ([1], [1], UNLIMITED, [hm.Restriction(vehicle=0, site=1)], "site"),
        ([1], [1], UNLIMITED, [hm.Restriction(vehicle=1, site=0)], "vehicle"),
        ([1], [1], UNLIMITED, [hm.Restriction(vehicle=-1, site=0)], "vehicle"),
    ],
)
def test_invalid_input_raises(demand, distances, vehicles, restrictions, message):
    with pytest.raises(ValueError, match=message):
        hm.clarke_wright(demand, distances, vehicles, restrictions)


def test_runs_are_immutable():
    run = hm.clarke_wright([1], [1], UNLIMITED).runs[0]

    with pytest.raises(dataclasses.FrozenInstanceError):
        run.distance = 0


def test_to_pandas():
    pytest.importorskip("pandas")

    demand, distances = random_problem(0)
    solution = hm.clarke_wright(demand, distances, UNLIMITED)
    frames = solution.to_pandas()

    assert list(frames["runs"].columns) == ["run", "vehicle", "max_load", "distance"]
    assert list(frames["sites"].columns) == ["site", "demand"]
    assert list(frames["visits"].columns) == ["run", "site", "order", "load"]

    assert len(frames["runs"]) == len(solution.runs)
    assert frames["sites"]["demand"].tolist() == demand.tolist()

    for run in solution.runs:
        visits = frames["visits"][frames["visits"]["run"] == run.id]
        assert tuple(visits.sort_values("order")["site"]) == run.sites


def test_to_pandas_explains_missing_pandas(monkeypatch):
    monkeypatch.setitem(sys.modules, "pandas", None)

    with pytest.raises(ImportError, match=r"heumilkr\[pandas\]"):
        hm.clarke_wright([1], [1], UNLIMITED).to_pandas()
