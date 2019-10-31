
#base
add_executable(asynclogging_bench1 AsyncLogging_bench1.cc)
target_link_libraries(asynclogging_bench1 matelib_base)

add_executable(asynclogging_bench2 AsyncLogging_bench2.cc)
target_link_libraries(asynclogging_bench2 matelib_base)

add_executable(logging_test Logging_test.cc)
target_link_libraries(logging_test matelib_base)

add_executable(thread_test Thread_test.cc)
target_link_libraries(thread_test matelib_base)

add_executable(threadlocal_test ThreadLocal_test.cc)
target_link_libraries(threadlocal_test matelib_base)

add_executable(threadpool_test ThreadPool_test.cc)
target_link_libraries(threadpool_test matelib_base)

add_executable(singleton_test Singleton_test.cc)
target_link_libraries(singleton_test matelib_base)

####
add_executable(eventloopthread_test EventLoopThread_test.cc)
target_link_libraries(eventloopthread_test matelib)

add_executable(tcpserver_test TcpServer_test.cc)
target_link_libraries(tcpserver_test matelib)

add_executable(timer_test Timer_test.cc)
target_link_libraries(timer_test matelib)

add_executable(tcpclient_test TcpClient_test.cc)
target_link_libraries(tcpclient_test matelib)