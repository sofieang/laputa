file(GLOB ZLIB_SRC "*.cpp")
target_sources(laputa PUBLIC ${ZLIB_SRC})
target_include_directories(laputa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
