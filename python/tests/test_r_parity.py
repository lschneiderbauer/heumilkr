"""The algorithm itself is tested in the R package (tests/testthat).

These tests make sure the Python package yields the same results: both use the
same C++ core, so a difference points to a problem in the way it is called. If
solver results change on purpose, regenerate the file with fixtures/generate.R.
"""

import json
from pathlib import Path

import pytest

import heumilkr as hm

SCENARIOS = json.loads(
    (Path(__file__).parent / "fixtures" / "r_results.json").read_text()
)


def as_list(value):
    # R's auto_unbox turns vectors of length one into a scalar
    return value if isinstance(value, list) else [value]


def solve(scenario):
    vehicles = scenario["vehicles"]
    restrictions = scenario["restrictions"]

    return hm.clarke_wright(
        as_list(scenario["demand"]),
        as_list(scenario["distances"]),
        [
            hm.Vehicle(capacity, None if count < 0 else count)
            for capacity, count in zip(
                as_list(vehicles["capacity"]), as_list(vehicles["count"])
            )
        ],
        [
            hm.Restriction(vehicle, site)
            for vehicle, site in zip(
                as_list(restrictions["vehicle"]), as_list(restrictions["site"])
            )
        ],
    )


@pytest.mark.parametrize("scenario", SCENARIOS, ids=lambda s: s["name"])
def test_solution_equals_r_solution(scenario):
    solution = solve(scenario)

    expected_runs = scenario["runs"]
    assert [run.id for run in solution.runs] == expected_runs["run"]
    assert [run.vehicle for run in solution.runs] == expected_runs["vehicle"]
    assert [run.max_load for run in solution.runs] == pytest.approx(
        expected_runs["max_load"]
    )
    assert [run.distance for run in solution.runs] == pytest.approx(
        expected_runs["distance"]
    )

    visits = [
        (run.id, order, site, load)
        for run in solution.runs
        for order, (site, load) in enumerate(zip(run.sites, run.loads))
    ]
    expected_visits = sorted(
        zip(
            scenario["visits"]["run"],
            scenario["visits"]["order"],
            scenario["visits"]["site"],
            scenario["visits"]["load"],
        )
    )
    assert [v[:3] for v in visits] == [v[:3] for v in expected_visits]
    assert [v[3] for v in visits] == pytest.approx([v[3] for v in expected_visits])

    assert solution.cost() == pytest.approx(scenario["cost"])
    assert solution.saving() == pytest.approx(scenario["saving"])
