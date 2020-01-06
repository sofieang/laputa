file(GLOB MINIZIP_SRC "*.c")
target_sources(laputa PUBLIC ${MINIZIP_SRC})
target_include_directories(laputa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
