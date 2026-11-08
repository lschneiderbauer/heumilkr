# heumilkr 0.4.0

-   First release of the Python package. It shares its C++ core with the R package of the same name: releases of both packages with the same major.minor version ship the same solver.

-   `clarke_wright()` solves the Capacitated Vehicle Routing Problem for multiple vehicle types with individual capacities and counts, `Restriction`s of vehicle types to sites, and negative ("backhaul") demands. Distances are accepted as a square matrix or in the condensed form of `scipy.spatial.distance.pdist()`.

-   The returned `Solution` lists its `Run`s with their sites, distance and maximal load, and provides `cost()` and `saving()`. Identical input always yields an identical solution.
