cmake_minimum_required(VERSION 3.18...3.31)
project(${SKBUILD_PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
  Python 3.10 REQUIRED
  COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)
find_package(nanobind CONFIG REQUIRED)

# All of src/ except the R binding, so that new core files are picked up
# without having to touch this file.
set(CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "${CORE_DIR}/*.cpp")
list(FILTER CORE_SOURCES EXCLUDE REGEX "/(cpp11|r_[^/]*)\\.cpp$")

nanobind_add_module(_core STABLE_ABI NB_STATIC bindings.cpp ${CORE_SOURCES})
target_include_directories(_core PRIVATE "${CORE_DIR}")

install(TARGETS _core LIBRARY DESTINATION heumilkr)
