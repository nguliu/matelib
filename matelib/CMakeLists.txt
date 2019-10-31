set(lib_SRCS
	Buffer.cc
	Channel.cc
	EpollPoller.cc
	EventLoop.cc
	EventLoopThread.cc
	EventLoopThreadPool.cc
	TcpConnection.cc
	TcpClient.cc
	TcpServer.cc
	TimerQueue.cc
)

add_library(matelib ${lib_SRCS})
target_link_libraries(matelib matelib_base)

install(TARGETS matelib DESTINATION lib)


set(HEADERS			#这里安装的头文件才对外部可见
	Buffer.h
	Channel.h
	EpollPoller.h
	EventLoop.h
	EventLoopThread.h
	EventLoopThreadPool.h
	InetAddress.h
	StringPiece.h
	TcpClient.h
	TcpConnection.h
	TcpServer.h
	Timer.h
	TimerQueue.h
	callbacks.h
)
install(FILES ${HEADERS} DESTINATION include/matelib/)

add_subdirectory(base)
add_subdirectory(tests)