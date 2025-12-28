# adding subdirectories
add_subdirectory(tasktree)
add_subdirectory(util)

# setting target sources
target_sources(Tasktree PRIVATE main.c)
set_target_properties(Tasktree PROPERTIES C_STANDARD 11)

target_sources(Test PRIVATE test.c)
set_target_properties(Test PROPERTIES C_STANDARD 11)

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
	PRIVATE
		unofficial::cunit::cunit
)
