add_library(Tasktreelib)

target_sources(Tasktreelib
	PRIVATE
		tasktree.c
	PUBLIC
		FILE_SET HEADERS
		FILES
			tasktree.h
)

target_link_libraries(Tasktreelib
	PUBLIC
		Util
)
