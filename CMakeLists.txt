cmake_minimum_required(VERSION 3.16)

project(laputa LANGUAGES C CXX VERSION 1.7)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Set up GSL
include(ExternalProject)
ExternalProject_Add(gsl
  GIT_REPOSITORY https://github.com/ampl/gsl.git
  GIT_TAG v2.5.0
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/gsl-prefix/install/ .
)

# Set up fltk
ExternalProject_Add(fltk
  GIT_REPOSITORY https://github.com/fltk/fltk.git
  GIT_TAG release-1.3.5
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/
    -DOPTION_BUILD_EXAMPLES=OFF
    -DOPTION_USE_SYSTEM_ZLIB=OFF
    -DOPTION_USE_SYSTEM_LIBPNG=OFF
    -DOPTION_USE_SYSTEM_LIBJPEG=OFF
    .
)

# Set up MuParserX
ExternalProject_Add(muparserx
  GIT_REPOSITORY https://github.com/beltoforion/muparserx.git
  GIT_TAG v4.0.8
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/muparserx-prefix/install/ .
)

# Create Laputa executable
if(UNIX)
  add_executable(laputa)
  set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-mavx")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-mavx")
endif(UNIX)
if(WIN32)
  add_executable(laputa WIN32 ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/Laputa.rc)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
endif(WIN32)
add_dependencies(laputa fltk gsl muparserx)
target_include_directories(laputa PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/gsl-prefix/install/include
  ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/include
  ${CMAKE_CURRENT_BINARY_DIR}/muparserx-prefix/install/include
)

# Link libraries
if(UNIX)
  find_library(LIB_dl dl)
  find_library(LIB_cairo cairo)
  find_library(LIB_freetype freetype)
  find_library(LIB_fontconfig fontconfig)
  find_library(LIB_fontconfig fontconfig)
  find_library(LIB_X11 X11)
  find_library(LIB_Xrender Xrender)
  find_library(LIB_Xext Xext)
  find_library(LIB_Xfixes Xfixes)
  find_library(LIB_Xcursor Xcursor)
  find_library(LIB_Xft Xft)
  find_library(LIB_Xinerama Xinerama)
  target_link_libraries(laputa
    ${CMAKE_CURRENT_BINARY_DIR}/gsl-prefix/install/lib/libgsl.a
    ${CMAKE_CURRENT_BINARY_DIR}/gsl-prefix/install/lib/libgslcblas.a
    ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/libfltk_images.a
    ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/libfltk_jpeg.a
    ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/libfltk_png.a
    ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/libfltk_z.a
    ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/libfltk.a
    ${CMAKE_CURRENT_BINARY_DIR}/muparserx-prefix/install/lib/libmuparserx.a
    ${LIB_dl}
    ${LIB_freetype}
    ${LIB_fontconfig}
    ${LIB_X11}
    ${LIB_Xrender}
    ${LIB_Xext}
    ${LIB_Xfixes}
    ${LIB_Xcursor}
    ${LIB_Xft}
    ${LIB_Xinerama}
  )
endif(UNIX)
if(WIN32)
  target_link_libraries(laputa
    ${CMAKE_CURRENT_BINARY_DIR}/gsl-prefix/install/lib/gsl.lib
    ${CMAKE_CURRENT_BINARY_DIR}/gsl-prefix/install/lib/gslcblas.lib
    ${CMAKE_CURRENT_BINARY_DIR}/muparserx-prefix/install/lib/muparserx.lib
    debug ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltk_imagesd.lib
    optimized ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltk_images.lib
    debug ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltk_jpegd.lib
    optimized ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltk_jpeg.lib
    debug ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltk_pngd.lib
    optimized ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltk_png.lib
    debug ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltk_zd.lib
    optimized ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltk_z.lib
    debug ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltkd.lib
    optimized ${CMAKE_CURRENT_BINARY_DIR}/fltk-prefix/install/lib/fltk.lib
  )

endif(WIN32)

# Copy data and docs directories
if(UNIX)
  file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  file(COPY docs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif(UNIX)
if(WIN32)
  file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
  file(COPY docs DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
  file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
  file(COPY docs DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
  file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo)
  file(COPY docs DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo)
endif(WIN32)

# Installation instructions
if(UNIX)
  install(TARGETS laputa
    DESTINATION /usr/local/bin/
  )
  install(DIRECTORY docs data
    DESTINATION /usr/local/share/laputa
  )
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/linux/laputa.desktop
    DESTINATION /usr/share/applications
  )
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/linux/laputa.png
    DESTINATION /usr/share/icons/hicolor/48x48/apps
  )
  set(CPACK_GENERATOR "DEB")
  set(CPACK_PACKAGE_NAME "Laputa installer")
  set(CPACK_PACKAGE_FILE_NAME "laputa-1.7-installer")
  set(CPACK_PACKAGE_DESCRIPTION "Install the Laputa social epistemology environment")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "dl, freetype, fontconfig, X11, Xrender, Xext, Xfixes, Xcursor, Xft, Xinerama")
  set(CPACK_PACKAGE_VERSION_MAJOR 1)
  set(CPACK_PACKAGE_VERSION_MINOR 7)
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "SA")
  include(CPack)
endif(UNIX)

# Recurse into subdirectories
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ui)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)
