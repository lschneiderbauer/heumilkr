#!/usr/bin/env bash
# Compares the performance of the C++ core between two commits by counting the
# instructions of benchmark/ci/bench.cpp under cachegrind. Instruction counts,
# unlike timings, hardly depend on the machine or on other load, and both
# commits are built with the same compiler in the same run.
#
# Usage: perf-compare.sh <base> <head>
#
# Commits, not the working tree, are compared: commit your changes first.
#
# Environment:
#   PERF_THRESHOLD  maximal accepted increase of instructions in percent (5)
#   PERF_ACCEPTED   "true" reports regressions as warnings only, for slowdowns
#                   that are intended
#
# Exits with 1 if a scenario got slower than the threshold allows. Writes a
# table to $GITHUB_STEP_SUMMARY if set, and to stdout.
set -euo pipefail

if [ $# -ne 2 ]; then
  echo "usage: $0 <base> <head>" >&2
  exit 2
fi

head=$(git rev-parse --verify "$2^{commit}")
# where the branch started, so that later changes of the base branch do not
# count towards the head
base=$(git merge-base "$(git rev-parse --verify "$1^{commit}")" "$head")
threshold=${PERF_THRESHOLD:-5}
accepted=${PERF_ACCEPTED:-false}
summary=${GITHUB_STEP_SUMMARY:-/dev/null}

scenarios=(small_100 uniform_2000 mixed_2000 fleet_1000)

repo=$(git rev-parse --show-toplevel)
work=$(mktemp -d)
trap 'git -C "$repo" worktree remove --force "$work/base" >/dev/null 2>&1 || true; git -C "$repo" worktree remove --force "$work/head" >/dev/null 2>&1 || true; rm -rf "$work"' EXIT

# the driver of the head is built against the core of both commits, so that
# both run exactly the same benchmark
driver="$work/bench.cpp"
git -C "$repo" show "$head:benchmark/ci/bench.cpp" >"$driver"

# returns 1 if the core of the commit can not be built with the driver
build() {
  local name=$1 sha=$2
  git -C "$repo" worktree add --detach --quiet "$work/$name" "$sha"
  local sources
  sources=$(find "$work/$name/src" -maxdepth 1 -name '*.cpp' \
    ! -name 'cpp11.cpp' ! -name 'r_*.cpp' | sort)
  # shellcheck disable=SC2086
  g++ -std=gnu++17 -O2 -g -DNDEBUG -I"$work/$name/src" \
    "$driver" $sources -o "$work/bench-$name"
}

build head "$head"
if ! build base "$base"; then
  msg="The benchmark driver can not be built against the base commit ${base:0:7} (changed API?), skipping the comparison."
  echo "::warning::$msg"
  printf '### Performance\n\n%s\n' "$msg" >>"$summary"
  exit 0
fi

# prints "<instructions> <checksum>"
measure() {
  local name=$1 scenario=$2
  local out="$work/cg-$name-$scenario.out"
  local checksum
  checksum=$(valgrind --tool=cachegrind --cache-sim=no --instr-at-start=no \
    --cachegrind-out-file="$out" --log-file=/dev/null \
    "$work/bench-$name" "$scenario")
  echo "$(awk '/^summary:/ { print $2 }' "$out") ${checksum// /_}"
}

table="| Scenario | Base (${base:0:7}) | Head (${head:0:7}) | Change | Same solution |
|---|--:|--:|--:|:-:|"
regressions=()

for scenario in "${scenarios[@]}"; do
  read -r base_ir base_sum <<<"$(measure base "$scenario")"
  read -r head_ir head_sum <<<"$(measure head "$scenario")"

  change=$(awk -v b="$base_ir" -v h="$head_ir" 'BEGIN { printf "%+.2f", (h - b) / b * 100 }')
  same=$([ "$base_sum" = "$head_sum" ] && echo yes || echo "**no**")
  flag=""
  if awk -v c="$change" -v t="$threshold" 'BEGIN { exit !(c > t) }'; then
    regressions+=("$scenario: $change% instructions")
    flag=" :warning:"
  fi

  table+=$'\n'"| $scenario | $base_ir | $head_ir | $change%$flag | $same |"
done

{
  echo "### Performance"
  echo
  echo "Instructions of the solver as counted by cachegrind, threshold +$threshold%."
  echo
  echo "$table"
  echo
} | tee -a "$summary"

if [ ${#regressions[@]} -eq 0 ]; then
  echo "No performance regression." | tee -a "$summary"
  exit 0
fi

for r in "${regressions[@]}"; do
  if [ "$accepted" = "true" ]; then
    echo "::warning title=Accepted performance regression::$r"
  else
    echo "::error title=Performance regression::$r"
  fi
done

if [ "$accepted" = "true" ]; then
  echo "Regression accepted (label performance-regression-accepted)." | tee -a "$summary"
  exit 0
fi

echo "Performance regression. If it is intended, add the label performance-regression-accepted to the pull request." | tee -a "$summary"
exit 1
