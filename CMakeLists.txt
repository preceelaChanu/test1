cmake_minimum_required(VERSION 3.16)
project(SmartGridBenchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set SEAL install directory
set(SEAL_DIR "${CMAKE_SOURCE_DIR}/SEAL/install/lib/cmake/SEAL-4.0")

# Find Microsoft SEAL
find_package(SEAL 4.0 REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# KeyGen executable
add_executable(keygen keygen/keygen.cpp)
target_link_libraries(keygen SEAL::seal)

# Client executable
add_executable(client client/client.cpp)
target_link_libraries(client SEAL::seal)

# Server executable
add_executable(server server/server.cpp)
target_link_libraries(server SEAL::seal)
