set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.10.0)
project(NewsAPI VERSION 0.1.0 LANGUAGES C CXX)

add_executable(NewsAPI hello.cpp)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 1.14.2) # Replace with your desired version from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

find_package(SQLite3 REQUIRED)

target_link_libraries(NewsAPI PRIVATE cpr::cpr nlohmann_json::nlohmann_json SQLite3::SQLite3)