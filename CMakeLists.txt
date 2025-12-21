set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
cmake_minimum_required(VERSION 3.23)
project(Tasktree C)

find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_subdirectory(src)
