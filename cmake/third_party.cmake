CPMAddPackage(
  NAME CLI11
  GITHUB_REPOSITORY CLIUtils/CLI11
  GIT_TAG v2.7.2
  OPTIONS "CLI11_MODULES ON"
)

set_target_properties(CLI11_Module PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
)

if(BUILD_TESTS)
  CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.18.0
    VERSION 1.18.0
    OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
  )
endif()
