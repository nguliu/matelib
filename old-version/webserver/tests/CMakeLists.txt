
#base
add_executable(asynclogging_bench1 AsyncLogging_bench1.cc)
target_link_libraries(asynclogging_bench1 webserver_base)

add_executable(asynclogging_bench2 AsyncLogging_bench2.cc)
target_link_libraries(asynclogging_bench2 webserver_base)

add_executable(logging_test Logging_test.cc)
target_link_libraries(logging_test webserver_base)

add_executable(thread_test Thread_test.cc)
target_link_libraries(thread_test webserver_base)

add_executable(threadpool_test ThreadPool_test.cc)
target_link_libraries(threadpool_test webserver_base)

add_executable(singleton_test Singleton_test.cc)
target_link_libraries(singleton_test webserver_base)

####
set(SRCS_Timer_test
	../Channel.cc
	../EpollPoller.cc
	../EventLoop.cc
	../TimerQueue.cc

	Timer_test.cc
)
add_executable(timer_test ${SRCS_Timer_test})
target_link_libraries(timer_test webserver_base)

set(SRCS_EventLoopThread_test
	../Channel.cc
	../EpollPoller.cc
	../EventLoop.cc
	../TimerQueue.cc
	../EventLoopThread.cc

	EventLoopThread_test.cc
)
add_executable(eventlooplhread_test ${SRCS_EventLoopThread_test})
target_link_libraries(eventlooplhread_test webserver_base)

set(SRCS_TcpServer_test
	../Channel.cc
	../Buffer.cc
	../EpollPoller.cc
	../EventLoop.cc
	../EventLoopThread.cc
	../EventLoopThreadPool.cc
	../TcpConnection.cc
	../TcpServer.cc
	../TimerQueue.cc

	TcpServer_test.cc
)
add_executable(tcpserver_test ${SRCS_TcpServer_test})
target_link_libraries(tcpserver_test webserver_base)