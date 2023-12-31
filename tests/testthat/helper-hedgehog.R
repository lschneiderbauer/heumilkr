gen.demand_net <- function(max_sites = 10L) {
  hedgehog::gen.and_then(
    hedgehog::gen.int(max_sites),
    function(n_sites) {
      hedgehog::gen.list
      list(
        demand = gen.demand(n_sites),
        distances = hedgehog::gen.with(
          gen.pos(n_sites),
          \(pos) dist(pos, method = "euclidean")
        )
      )
    }
  )
}

gen.demand <- function(n_sites) {
  hedgehog::gen.c(hedgehog::gen.unif(1, 33, shrink.median = FALSE), of = n_sites)
}

gen.pos <- function(n_sites) {
  hedgehog::gen.with(
    list(
      x = gen.coords(n_sites),
      y = gen.coords(n_sites)
    ),
    function(l) {
      data.frame(
        pos_x = l$x,
        pos_y = l$y
      )
    }
  )
}

gen.coords <- function(n_sites) {
  # we want to generate coordinates over R^2 with a hole at the origin,
  # since the coordinates should not coincide with the position of the
  # supplying site at (0,0).
  hedgehog::gen.with(
    list(
      positives = hedgehog::gen.c(
        hedgehog::gen.unif(1, 10, shrink.median = FALSE),
        of = n_sites
      ),
      half_plane = hedgehog::gen.element(c(1, -1))
    ),
    \(g) c(0, g$half_plane * g$positives)
  )
}
