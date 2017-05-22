cmake_minimum_required(VERSION 2.8.3)
project(regression_experiments)

find_package(catkin REQUIRED COMPONENTS
  rosban_fa
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES regression_experiments
  CATKIN_DEPENDS rosban_fa
)

###########
## Build ##
###########

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  )

# Directories containing sources files
set(DIRECTORIES
  src/regression_experiments/
  )

# Build ALL_SOURCES
set (SOURCES)
set (ALL_SOURCES)
set (PREFIXED_SOURCES)
foreach (DIRECTORY ${DIRECTORIES})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}")
    include (${DIRECTORY}/sources.cmake)

    foreach (SOURCE ${SOURCES})
        set (PREFIXED_SOURCES ${PREFIXED_SOURCES} ${DIRECTORY}/${SOURCE})
    endforeach (SOURCE)

    set (ALL_SOURCES ${ALL_SOURCES} ${PREFIXED_SOURCES})
endforeach (DIRECTORY)

# Declare the library
add_library(regression_experiments ${ALL_SOURCES} )
target_link_libraries(regression_experiments ${catkin_LIBRARIES} )

# Declare the binaries
add_executable(test_gp_approximations src/test_gp_approximations.cpp)
target_link_libraries(test_gp_approximations
  regression_experiments
  ${catkin_LIBRARIES}
  )

add_executable(test_1d_regression src/test_1d_regression.cpp)
target_link_libraries(test_1d_regression
  regression_experiments
  ${catkin_LIBRARIES}
  )

add_executable(benchmark_regression src/benchmark_regression.cpp)
target_link_libraries(benchmark_regression
  regression_experiments
  ${catkin_LIBRARIES}
  )

add_executable(build_forests_predictions src/build_forests_predictions.cpp)
target_link_libraries(build_forests_predictions
  regression_experiments
  ${catkin_LIBRARIES}
  )
