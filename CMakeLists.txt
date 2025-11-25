cmake_minimum_required(VERSION 3.5)
project(tasktree C)
find_package(SQLite3 REQUIRED)
set(SOURCE_FILES
	main.c
)
add_executable(tasktree ${SOURCE_FILES})
set_target_properties(tasktree PROPERTIES C_STANDARD 11)
target_link_libraries(tasktree PRIVATE SQLite::SQLite3)
