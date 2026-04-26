cmake_minimum_required(VERSION 3.16)
project(AuthService LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Drogon CONFIG REQUIRED)

add_executable(AuthService main.cpp)
target_link_libraries(AuthService PRIVATE Drogon::Drogon)
