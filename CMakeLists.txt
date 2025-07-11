cmake_minimum_required(VERSION 3.31)
project(untitled1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

set(BOOST_ROOT "C:/boost_1_88_0")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/boost")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib64-msvc-14.1")

set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Link Boost statically")
set(Boost_USE_MULTITHREADED ON)
set(Boost_DEBUG ON)

add_definitions(-DBOOST_ALL_NO_LIB)

find_package(Boost 1.88 REQUIRED COMPONENTS filesystem system)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost не найден!")
endif()

message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libs: ${Boost_LIBRARIES}")

add_executable(untitled1
        main.cpp
        app.rc
        notrat_.cpp
        notrat_.h
)

target_include_directories(untitled1 PRIVATE ${Boost_INCLUDE_DIRS})
target_link_directories(untitled1 PRIVATE ${BOOST_LIBRARYDIR})
target_link_libraries(untitled1 PRIVATE ${Boost_LIBRARIES})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(untitled1 PRIVATE -static-libstdc++ -static-libgcc)
endif()