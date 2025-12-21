# adding executables
add_executable(Tasktree)

target_sources(Tasktree PRIVATE main.c)
set_target_properties(Tasktree PROPERTIES C_STANDARD 11)

add_executable(Test)

target_sources(Test PRIVATE test.c)
set_target_properties(Test PROPERTIES C_STANDARD 11)

# adding subdirectories
add_subdirectory(tasktree)
add_subdirectory(util)

# linking executables
target_link_libraries(Tasktree
	PRIVATE
		Tasktreelib
)

target_link_libraries(Test
	PRIVATE
		Util
	PRIVATE
		Tasktreelib
)
