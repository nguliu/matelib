#set(SRCS
#
#) 
#
#include_directories(${PROJECT_SOURCE_DIR}/base)
#
#add_executable(webserver ${SRCS})
#target_link_libraries(webserver webserver_base)

add_subdirectory(base)
add_subdirectory(tests)
