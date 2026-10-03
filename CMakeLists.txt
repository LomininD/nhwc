cmake_minimum_required(VERSION 3.28)
project(nhwc LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(caches)
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
target_include_directories(caches
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(caches PUBLIC cxx_std_23)

add_executable(main source/main.cpp)
target_sources(main
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
            source/config.cppm
            source/util.cppm
)
target_compile_features(main PUBLIC cxx_std_23)

target_link_libraries(main caches)

enable_testing()
add_subdirectory(tests)
