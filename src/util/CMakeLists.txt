add_library(Util)

target_sources(Util
	PRIVATE
		util.c
	PUBLIC
		FILE_SET HEADERS
		FILES
			util.h
)

target_link_libraries(Util
	PRIVATE
		unofficial::sqlite3::sqlite3
)
