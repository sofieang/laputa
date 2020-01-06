file(GLOB TINYXML_SRC "*.cpp")
target_sources(laputa PUBLIC ${TINYXML_SRC})
target_include_directories(laputa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
