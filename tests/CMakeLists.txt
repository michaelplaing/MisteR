# Testing library
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.6
)
FetchContent_MakeAvailable(Catch2)
# Adds Catch2::Catch2

include_directories(mister PUBLIC ${mister_SOURCE_DIR}/include)

# Tests need to be added as executables first
add_executable(simpletest simpletest.cpp)
# I'm using C++17 in the test
target_compile_features(simpletest PRIVATE cxx_std_17)

# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(simpletest PRIVATE mister Catch2::Catch2)

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME simpletesttest COMMAND simpletest) # Command can be a target

add_subdirectory(unittests)
