set(lib_SRCS
	Buffer.cc
	Channel.cc
	EpollPoller.cc
	EventLoop.cc
	EventLoopThread.cc
	EventLoopThreadPool.cc
	TcpConnection.cc
	TcpServer.cc
	TimerQueue.cc
)

add_library(webserver_lib ${lib_SRCS})
target_link_libraries(webserver_lib webserver_base)

add_subdirectory(base)

add_subdirectory(http)
add_subdirectory(tests)