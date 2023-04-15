cmake_minimum_required(VERSION 3.5)

project(psemulator)


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -O3")


SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRC "src/*.c" "src/*.h")
add_executable(psemulator ${SRC})

set(SDL2_BUILDING_LIBRARY ON)
find_package(SDL2 REQUIRED)
include_directories(psemulator PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(psemulator PRIVATE ${SDL2_LIBRARIES})
