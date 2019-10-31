set(base_SRCS
	AsyncLogging.cc
	LogFile.cc
	Logging.cc
	LogStream.cc
	Thread.cc
	ThreadPool.cc
	Timestamp.cc
)

add_library(matelib_base ${base_SRCS})
target_link_libraries(matelib_base pthread rt)

install(TARGETS matelib_base DESTINATION lib)

file(GLOB HEADERS "*.h")	#这里表示安装所有的头文件到库中
install(FILES ${HEADERS} DESTINATION include/matelib/base)