#!/usr/bin/env bash
# The R and Python package share major.minor, which identifies the state of the
# C++ core in src/: every change there requires a minor bump of both packages.
# The patch level is free, for releases that touch only one package's wrapper.
set -euo pipefail

r_version=$(sed -n 's/^Version:[[:space:]]*//p' DESCRIPTION)
py_version=$(sed -n 's/^version = "\(.*\)"$/\1/p' pyproject.toml)

if [ "$(cut -d. -f1-2 <<< "$r_version")" != "$(cut -d. -f1-2 <<< "$py_version")" ]; then
  echo "::error::Major.minor of the R package ($r_version, DESCRIPTION) and the Python package ($py_version, pyproject.toml) differ."
  exit 1
fi
echo "R $r_version and Python $py_version share major.minor."
