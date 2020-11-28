cmake_minimum_required(VERSION 3.10) 
project(fiocca LANGUAGES CXX)

# We use C++20 by defaults to test some interesting new features.
# Anyway we still try to make compatibility to C++17.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -O3 -fconcepts")

set(FIOCCA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# Note that OpenCV is not required in our framework. If OpenCV package is
# successfully found, a preprocess macro named FIOCCA_OPENCV_AVAILABLE_ will
# be defined and thus you can program accordingly.
find_package(OpenCV QUIET)
if (OpenCV_FOUND)
  add_compile_definitions(FIOCCA_OPENCV_AVAILABLE_)
endif()

# Find OpenMP. If OpenMP is enabled, we optimize some for loops in a simple
# way with OpenMP parallel.
find_package(OpenMP QUIET)
if (OpenMP_FOUND)
  add_compile_definitions(FIOCCA_OPENMP_AVAILABLE_)
endif()

# Include required headers.
include_directories(${FIOCCA_INCLUDE_DIR})

# Specify all cpp files in the source directory as sources for convenience.
file(GLOB_RECURSE FIOCCA_SRCS src/*.cpp)
add_library(fiocca SHARED ${FIOCCA_SRCS})
if (OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries(fiocca ${OpenCV_LIBRARIES})
endif()

# If the compiler supports OpenMP, Import OpenMP modules for CXX.
if (OpenMP_FOUND)
  target_link_libraries(fiocca OpenMP::OpenMP_CXX)
endif()

# Example build flags that defaults to be opened.
set(FIOCCA_BUILD_EXAMPLES ON)
set(FIOCCA_EXAMPLE_DIR ${PROJECT_SOURCE_DIR}/example)
if (FIOCCA_BUILD_EXAMPLES)
  add_executable(
    edist_example
    ${FIOCCA_EXAMPLE_DIR}/edist.cpp)
  target_link_libraries(edist_example fiocca)
endif()

# Install settings.
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)
install(DIRECTORY include/ DESTINATION include)
install(TARGETS fiocca DESTINATION lib)

