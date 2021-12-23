cmake_minimum_required(VERSION 3.0)

project(gb)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(fmt CONFIG REQUIRED)
file(GLOB SOURCE {
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
})

add_executable(gb ${SOURCE})
target_link_libraries(gb PRIVATE fmt::fmt)