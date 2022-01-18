find_package(PkgConfig REQUIRED)
pkg_check_modules (Catch2 REQUIRED catch2>=2.13.8)
find_package(Catch2 REQUIRED)
### alternative to having catch2 installed:
#FetchContent_Declare(
#  Catch2
#  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#  GIT_TAG v2.13.8
#)
#FetchContent_MakeAvailable(Catch2)

include_directories(mister PUBLIC ${mister_SOURCE_DIR}/include)

# Tests need to be added as executables first - include CatchMain
add_executable(simpletest simpletest.cpp CatchMain.cpp)
add_executable(init_connect_packet init_connect_packet.cpp CatchMain.cpp)

# Use C++17 in the test
target_compile_features(simpletest PRIVATE cxx_std_17)
target_compile_features(init_connect_packet PRIVATE cxx_std_17)

# linked to the main library, as well as the Catch2 testing library
target_link_libraries(simpletest PRIVATE Catch2::Catch2)
target_link_libraries(init_connect_packet PRIVATE mister Catch2::Catch2)

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME simpletest COMMAND simpletest) # Command can be a target
add_test(NAME init_connect_packet COMMAND init_connect_packet) # Command can be a target

add_subdirectory(unittests)
