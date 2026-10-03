cmake_minimum_required(VERSION 3.20)
project(qem_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(pybind11 CONFIG REQUIRED)

include_directories(include)

add_library(qem_core STATIC src/qem_engine.cpp)

pybind11_add_module(qem_engine_cpp python/binding.cpp)
target_link_libraries(qem_engine_cpp PRIVATE qem_core pybind11::module)
