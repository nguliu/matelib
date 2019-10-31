set(SRCS_HttpServer_test
	HttpContext.cc
	HttpServer.cc

	HttpServer_test.cc
)

add_executable(httpserver_test ${SRCS_HttpServer_test})
target_link_libraries(httpserver_test webserver_lib)