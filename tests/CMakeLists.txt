CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.18.0
    VERSION 1.18.0
    OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
)

add_executable(
    nhwc_test
    arc_cache_test.cpp
    lru_cache_test.cpp
    two_queue_cache_test.cpp
    lfu_cache_test.cpp
    lirs_cache_test.cpp
    belady_cache_test.cpp
)

target_link_libraries(
    nhwc_test
    PRIVATE
    caches
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(nhwc_test)
