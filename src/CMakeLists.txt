add_executable(Tasktree)

target_sources(Tasktree PRIVATE main.c)
set_target_properties(Tasktree PROPERTIES C_STANDARD 11)

add_subdirectory(tasktree)
add_subdirectory(util)

target_link_libraries(Tasktree
	PRIVATE
		SQLite::SQLite3
	PRIVATE
		Tasktreelib
)
