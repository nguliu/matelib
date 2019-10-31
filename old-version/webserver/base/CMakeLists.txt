set(base_SRCS
	AsyncLogging.cc
	Condition.cc
	CountDownLatch.cc
	LogFile.cc
	Logging.cc
	LogStream.cc
	Thread.cc
	ThreadPool.cc
	Timestamp.cc
)

add_library(webserver_base ${base_SRCS})
target_link_libraries(webserver_base pthread rt)