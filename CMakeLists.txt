# find_package(PkgConfig REQUIRED)
#
# if ( NOT (PKGCONFIG_FOUND))
#       message(FATAL_ERROR "Please Install PPkgConfig: CMake will Exit")
# endif()
# pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.8 gstreamer-app-1.0>=1.8 gobject-2.0 glib-2.0 )
# #pkg_check_modules(GstApp REQUIRED gstreamer-app-1.0)
# if ( NOT (GST_FOUND))
#       message(FATAL_ERROR "Please Install Gstreamer Dev: CMake will Exit")
# endif()
# set(ENV{PKG_CONFIG_PATH})
cmake_minimum_required(VERSION 3.13)

project(gstprojectm)
set(TARGET_NAME "gstprojectm")

set(OpenGL_GL_PREFERENCE "GLVND")

#set(PKG_CONFIG_PATH "/home/tristan/Documents/pm4.0/usr/lib/pkgconfig:${PKG_CONFIG_PATH}")

find_package(PkgConfig REQUIRED)
#find_package(gstreamer REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.8 gstreamer-base-1.0>=1.8 gstreamer-video-1.0>=1.8 gstreamer-audio-1.0>=1.8 gstreamer-gl-1.0 gstreamer-pbutils-1.0)

pkg_check_modules(GLIB2 glib-2.0 REQUIRED)
#set(ENV{PKG_CONFIG_PATH})

if(NOT projectM4_DIR)
        set(projectM4_DIR "/home/tristan/Documents/pm4.0/usr/lib/cmake/projectM4")
endif()
#set(libprojectM_DIR "/home/tristan/Documents/pm4.0/usr/lib/cmake/projectM4")#/home/tristan/dev/pm/projectm/build/src/libprojectM/)
#set(CMAKE_PREFIX_PATH "/home/tristan/dev/")
find_package(projectM4 REQUIRED PATHS "${projectM4_DIR}")
#find_library(projectM-4_DIR projectM-4 REQUIRED PATHS "/home/tristan/Documents/pm4.0/usr/lib")
#find_package(libprojectM REQUIRED PATHS "${projectM4_DIR}")#/home/tristan/dev/pm/projectm/build/src/libprojectM/)
message(glib include dirs: "${GLIB2_INCLUDE_DIRS}")
message(glib libs: "${GLIB2_LIBRARIES}")

file(GLOB MAIN_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/*.c
        )

file(GLOB MAIN_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        )
add_compile_options(-O3)
add_library(${TARGET_NAME} SHARED ${MAIN_SRC} ${MAIN_HEADERS})

target_include_directories(${TARGET_NAME}
        PUBLIC
        "${GST_INCLUDE_DIRS}"
        ${GSTREAMER_BASE_INCLUDE_DIRS}
        ${GSTREAMER_AUDIO_INCLUDE_DIRS}
        ${GLIB2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
       # "/home/tristan/Documents/pm4.0/usr/include/projectM-4"
        )

message(GST include dirs: "${GST_INCLUDE_DIRS}")
message(GST libraries: "${GST_LIBRARIES}")

message(projectm: "${projectM4}")

target_link_libraries(${TARGET_NAME}
        PUBLIC
        "${GST_LIBRARIES}"
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_BASE_LIBRARIES}
        ${GSTREAMER_VIDEO_LIBRARIES}
        ${GSTREAMER_GL_LIBRARIES}
        ${GSTREAMER_PBUTILS_LIBRARIES}
        ${GSTREAMER_AUDIO_LIBRARIES}
        ${GLIB2_LIBRARIES}
        PRIVATE
        libprojectM::projectM
        )
