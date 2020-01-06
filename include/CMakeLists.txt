file(GLOB LAPUTA_INCLUDES "*.h")
target_sources(laputa PUBLIC ${LAPUTA_INCLUDES})
target_include_directories(laputa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
