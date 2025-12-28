set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
cmake_minimum_required(VERSION 3.23)
project(Tasktree C)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(unofficial-cunit CONFIG REQUIRED)

# adding executables
add_executable(Tasktree)
set_target_properties(Tasktree PROPERTIES C_STANDARD 11)

add_executable(Test)
set_target_properties(Test PROPERTIES C_STANDARD 11)

add_subdirectory(src)
