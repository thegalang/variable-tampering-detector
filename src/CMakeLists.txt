add_library(avg_bb_size SHARED avg_bb_size.c)
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
	message(FATAL_ERROR "DynamoRIO package required to build")

endif(NOT DynamoRIO_FOUND)
configure_DynamoRIO_client(avg_bb_size)