set(rpc_SRCS
	RpcClient.cc
	RpcServer.cc
)
add_library(matelib_rpc ${rpc_SRCS})
target_link_libraries(matelib_rpc matelib)

install(TARGETS matelib_rpc DESTINATION lib)


set(HEADERS
	RpcClient.h
	RpcServer.h
	Serializer.h
)
install(FILES ${HEADERS} DESTINATION include/matelib/rpc/)



add_executable(rpcclient_test RpcClient_test.cc)
target_link_libraries(rpcclient_test matelib_rpc)

add_executable(rpcserver_test RpcServer_test.cc)
target_link_libraries(rpcserver_test matelib_rpc)