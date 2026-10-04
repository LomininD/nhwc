cmake_minimum_required(VERSION 3.28)
project(nhwc LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CPM.cmake)

add_library(caches STATIC)
target_sources(caches
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
            source/arc_cache.cppm
            source/lirs_cache.cppm
            source/two_queue_cache.cppm
            source/lfu_cache.cppm
            source/lru_cache.cppm
            source/multi_level_cache.cppm
            source/base_cache.cppm
            source/lru_queue.cppm
            source/belady_cache.cppm
)
target_compile_features(caches PUBLIC cxx_std_23)

include(cmake/CLI11.cmake)
find_package(CLI11 REQUIRED)

add_executable(${PROJECT_NAME} source/main.cpp)
target_sources(${PROJECT_NAME}
    PRIVATE
        FILE_SET CXX_MODULES
        FILES
            source/config.cppm
            source/util.cppm
)
target_link_libraries(${PROJECT_NAME} PRIVATE caches CLI11::Module)

add_executable(ideal_cache source/ideal_cache.cpp)
target_sources(ideal_cache
    PRIVATE
        FILE_SET CXX_MODULES
        FILES
            source/belady_cache.cppm
            source/util.cppm
)
target_compile_features(ideal_cache PUBLIC cxx_std_23)

foreach(target ${PROJECT_NAME} ideal_cache)
    target_compile_options(${target}
        PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/Wall>
            $<$<CXX_COMPILER_ID:GNU,Clang>: -Wall -Wextra -pedantic>
    )
endforeach()

enable_testing()
add_subdirectory(tests)
