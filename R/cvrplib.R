cvrplib_instance <- function(name, type, comment = "", dimension, capacity,
                             pos, demand, optimum) {
  new_cvrplib_instance(
    name, type, comment, dimension, capacity,
    pos, demand, optimum
  )
}

new_cvrplib_instance <- function(name, type, comment, dimension, capacity,
                                 pos, demand, optimum) {
  stopifnot(is.character(name))
  stopifnot(is.character(type))
  stopifnot(is.character(comment))
  stopifnot(is.numeric(dimension))
  stopifnot(is.numeric(capacity))
  stopifnot(is.data.frame(pos))
  stopifnot(is.data.frame(demand))
  stopifnot(is.numeric(optimum))

  structure(
    list(
      name = name,
      type = type,
      comment = comment,
      dimension = dimension,
      capacity = capacity,
      pos = pos,
      demand = demand,
      optimum = optimum
    ),
    class = "cvrplib_instance"
  )
}

#' @import cli
#' @export
format.cvrplib_instance <- function(x, ...) {
  url <-
    paste0(
      "http://vrp.atd-lab.inf.puc-rio.br/index.php/en/plotted-instances?data=",
      x$name
    )

  cli_format_method({
    cli_h1("CVRPLIB data {cli::symbol$line} {x$name}")
    cli_ul(c(
      "Number of sites: {.val {x$dimension}}",
      "Vehicle capacity: {.val {x$capacity}}",
      "Optimal distance: {.val {x$optimum}}",
      "Demand: ({.val {x$demand$demand}})",
      "Comment: '{x$comment}'"
    ))
    cli_text("See more at {.href [vrp.atd-lab.inf.puc-rio.br]({url})}.")
  })
}

#' @export
print.cvrplib_instance <- function(x, ...) {
  cat(format(x, ...), sep = "\n")
}

#' Apply [clarke_wright()] to CVRPLIB data
#'
#' @param instance
#'  A "`cvrplib_instance`" object. See [cvrplib_download()] or bundled CVRPLIB data
#'  like [cvrplib_A].
#'
#' @return
#'  A "`heumilkr_solution`" object. See [clarke_wright()].
#'
#' @family cvrplib
#' @concept cvrplib
#'
#' @examples
#' clarke_wright_cvrplib(cvrplib_A[[1]])
#'
#' @export
clarke_wright_cvrplib <- function(instance) {
  stopifnot(inherits(instance, "cvrplib_instance"))

  merged <-
    merge(
      instance$pos,
      instance$demand,
      by = "site"
    )

  origin <-
    instance$pos[instance$pos$site == setdiff(instance$pos$site, instance$demand$site), c("x", "y")]

  demand <- merged$demand
  dist <- dist(rbind(data.frame(x = origin$x, y = origin$y), merged[, c("x", "y")]))

  clarke_wright(demand, dist,
    vehicles = data.frame(n = NA_integer_, caps = instance$capacity)
  )
}

cvrplib_clarke_wright_perf <- function(cvrplib_instance) {
  stopifnot(inherits(cvrplib_instance, "cvrplib_instance"))

  milkr_perf_scale(
    clarke_wright_cvrplib(cvrplib_instance),
    cvrplib_instance$optimum
  )
}

cvrplib_clarke_wright_perf2 <- function(cvrplib_instance) {
  stopifnot(inherits(cvrplib_instance, "cvrplib_instance"))

  milkr_perf_rel(
    clarke_wright_cvrplib(cvrplib_instance),
    cvrplib_instance$optimum
  )
}

cvrplib_url <- "http://vrp.atd-lab.inf.puc-rio.br/media/com_vrp/instances/"


#' List available CVRPLIB online data
#'
#' Scrapes the CVRPLIB website to look for available data sets.
#' This function call can take some time.
#'
#' @return
#'  A vector of data set qualifiers which can be used with [cvrplib_download()].
#'
#' @family cvrplib
#' @concept cvrplib
#' @export
cvrplib_ls <- function() {
  # reading directories first
  rel_dirs <-
    setdiff(
      xml2::xml_attr(
        xml2::xml_find_all(
          xml2::read_html(paste0(readLines(url(cvrplib_url)), collapse = "\n")),
          "//a[substring(@href, string-length(@href) - string-length('/') +1) = '/']"
        ),
        "href"
      ),
      c("/media/com_vrp/", "test/", "CMT/")
    )
  # CMT has buggy .sol files

  # iterate over all directories and collect file names
  vrp_files <-
    grep(
      ".vrp$",
      unlist(
        lapply(
          rel_dirs,
          function(rel_dir) {
            paste0(
              rel_dir,
              xml2::xml_attr(
                xml2::xml_find_all(
                  xml2::read_html(
                    paste0(
                      readLines(url(paste0(cvrplib_url, rel_dir))),
                      collapse = "\n"
                    )
                  ),
                  "//a[substring(@href, string-length(@href) - string-length('.vrp') +1) = '.vrp']"
                ),
                "href"
              )
            )
          }
        )
      ),
      value = TRUE
    )

  substr(vrp_files, 1, nchar(vrp_files) - 4)
}

extract_header <- function(content, header) {
  regexp <- paste0("^", toupper(header), "\\s*:\\s*(.+?)\\s*$")

  unlist(regmatches(content, regexec(regexp, content)))[[2]]
}
extract_cost <- function(content) {
  regexp <- paste0("^Cost\\s*(.*)$")

  unlist(regmatches(content, regexec(regexp, content)))[[2]]
}

#' CVRPLIB problem instance downloader
#'
#' [CVRLIB](http://vrp.atd-lab.inf.puc-rio.br/) offers a selection of
#' CVRP problem instances. This function downloads the instance data and
#' conveniently makes it available to be fed into solver functions, e.g. with
#' [clarke_wright_cvrplib()]. The primary purpose for those instances is
#' benchmarking / comparing speed as well as performance of solvers.
#'
#' @param qualifier
#'  The qualifier of the problem instance. E.g. "tai/tai150d".
#'  This can either be inferred directly from the website or by the output of
#'  [cvrplib_ls()].
#'
#' @return
#'  Returns a "`cvrplib_instance`" object which contains CVRPLIB problem
#'  instance data.
#'
#' @family cvrplib
#' @concept cvrplib
#' @importFrom utils read.csv
#' @importFrom stats dist
#' @export
cvrplib_download <- function(qualifier) {
  stopifnot(is.character(qualifier))

  content <- readLines(url(paste0(cvrplib_url, qualifier, ".vrp")))

  name <- extract_header(content, "NAME")
  comment <- extract_header(content, "COMMENT")
  type <- extract_header(content, "TYPE")
  dimension <- as.numeric(extract_header(content, "DIMENSION"))
  capacity <- as.numeric(extract_header(content, "CAPACITY"))

  edge_weight_type <- extract_header(content, "EDGE_WEIGHT_TYPE")

  if (edge_weight_type %in% c("EUC_2D", "GEO")) {
    coord_section <- grep("NODE_COORD_SECTION", content, fixed = TRUE) + 1

    pos <-
      read.csv(
        text = content[coord_section:(coord_section + dimension - 1)],
        col.names = c("site", "x", "y"),
        sep = "",
        header = FALSE
      )
    pos$x <- as.numeric(pos$x)
    pos$y <- as.numeric(pos$y)
  } else if (edge_weight_type == "EXPLICIT") {
    edge_weight_format <- extract_header(content, "EDGE_WEIGHT_FORMAT")
    edge_weight_section <- grep("EDGE_WEIGHT_SECTION", content, fixed = TRUE) + 1

    if (edge_weight_format == "LOWER_DIAG_ROW") {
      demand_section <- grep("DEMAND_SECTION", content, fixed = TRUE)
      edge_content <- content[edge_weight_section:(demand_section - 1)]

      lower_triang <- as.numeric(unlist(lapply(strsplit(edge_content, "\\s+"), function(x) x[x != ""])))
      dmat <- matrix(0, nrow = dimension, ncol = dimension)
      dmat[upper.tri(dmat, diag = TRUE)] <- lower_triang
      pos <- as.data.frame(cmdscale(dist(dmat + t(dmat))))
      colnames(pos) <- c("x", "y")
      pos$site <- 1:nrow(pos)
    } else if (edge_weight_format == "LOWER_ROW") {
      demand_section <- grep("DEMAND_SECTION", content, fixed = TRUE)
      edge_content <- content[edge_weight_section:(demand_section - 1)]

      lower_triang <- as.numeric(unlist(lapply(strsplit(edge_content, "\\s+"), function(x) x[x != ""])))
      dmat <- matrix(0, nrow = dimension, ncol = dimension)
      dmat[upper.tri(dmat)] <- lower_triang
      pos <- as.data.frame(cmdscale(dist(dmat + t(dmat))))
      colnames(pos) <- c("x", "y")
      pos$site <- 1:nrow(pos)
    } else if (edge_weight_format == "FULL_MATRIX") {
      edge_content <- content[edge_weight_section:(edge_weight_section + dimension - 1)]

      dmat <- as.numeric(unlist(lapply(strsplit(edge_content, "\\s+"), function(x) x[x != ""])))
      dim(dmat) <- c(dimension, dimension)

      pos <- as.data.frame(cmdscale(dist(dmat)))
      colnames(pos) <- c("x", "y")
      pos$site <- 1:nrow(pos)
    } else {
      stop(paste0("Unknown edge weight format '", edge_weight_format, "'."))
    }
  } else {
    stop(paste0("Unknown edge weight type '", edge_weight_type, "'."))
  }


  demand_section <- grep("DEMAND_SECTION", content, fixed = TRUE) + 1

  demand <-
    read.csv(
      text = content[demand_section:(demand_section + dimension - 1)],
      sep = "",
      header = FALSE,
      col.names = c("site", "demand", "tmp")
    )[, c("site", "demand")]
  demand$demand <- as.numeric(demand$demand)

  demand <- demand[demand$demand > 0, ]

  ## solution file
  optimum <-
    as.numeric(
      extract_cost(readLines(url(paste0(cvrplib_url, qualifier, ".sol"))))
    )

  cvrplib_instance(
    name, type, comment, dimension, capacity,
    pos, demand, optimum
  )
}
